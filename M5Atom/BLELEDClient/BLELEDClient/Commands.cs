namespace BLELEDClient;

using System;
using System.Text;

using Smart.CommandLine.Hosting;

using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using Windows.Devices.Enumeration;
using Windows.Storage.Streams;

public static class CommandBuilderExtensions
{
    public static void AddCommands(this ICommandBuilder commands)
    {
        commands.AddCommand<LedCommand>();
    }
}

[Command("led", "LED")]
public sealed class LedCommand : ICommandHandler
{
    private static readonly Guid ServiceUuid = Guid.Parse("6E400001-B5A3-F393-E0A9-E50E24DCCA9E"); // UART Service
    private static readonly Guid TxCharUuid = Guid.Parse("6E400002-B5A3-F393-E0A9-E50E24DCCA9E"); // UART TX(Write)

    [Option<string>("--address", "-a", Description = "Address", Required = true)]
    public string Address { get; set; } = default!;

    [Option<byte>("--red", "-r", Description = "Red", DefaultValue = 0)]
    public byte Red { get; set; } = default!;

    [Option<byte>("--green", "-g", Description = "Green", DefaultValue = 0)]
    public byte Green { get; set; } = default!;

    [Option<byte>("--blue", "-b", Description = "Blue", DefaultValue = 0)]
    public byte Blue { get; set; } = default!;

    public async ValueTask ExecuteAsync(CommandContext context)
    {
        var address = Address.Replace(":", "").Replace("-", "").Trim();
        if (address.Length != 12)
        {
            Console.WriteLine("Invalid address format.");
            return;
        }

        using var device = await BluetoothLEDevice.FromBluetoothAddressAsync(Convert.ToUInt64(address, 16));
        if (device == null)
        {
            Console.WriteLine("Bluetooth connect failed.");
            return;
        }

        var servicesResult = await device.GetGattServicesAsync(BluetoothCacheMode.Uncached);
        if (servicesResult.Status != GattCommunicationStatus.Success)
        {
            Console.WriteLine($"Get services failed. Status={servicesResult.Status}");
            return;
        }

        using var service = servicesResult.Services.FirstOrDefault(s => s.Uuid == ServiceUuid);
        if (service == null)
        {
            Console.WriteLine("UART service not found.");
            return;
        }

        var access = await service.RequestAccessAsync();
        if (access != DeviceAccessStatus.Allowed)
        {
            Console.WriteLine($"Request access failed. Status={access}");
            return;
        }

        var charsResult = await service.GetCharacteristicsAsync(BluetoothCacheMode.Uncached);
        if (charsResult.Status != GattCommunicationStatus.Success)
        {
            Console.WriteLine($"Get characteristics failed. Status={charsResult.Status}");
            return;
        }

        var tx = charsResult.Characteristics.FirstOrDefault(c => c.Uuid == TxCharUuid);
        if (tx == null)
        {
            Console.WriteLine("TX characteristic not found.");
            return;
        }

        var cmd = $"RGB {Red} {Green} {Blue}\n";
        var bytes = Encoding.ASCII.GetBytes(cmd);

        using var writer = new DataWriter();
        writer.WriteBytes(bytes);
        var buffer = writer.DetachBuffer();

        var writeStatus = await tx.WriteValueAsync(buffer, GattWriteOption.WriteWithoutResponse);
        if (writeStatus != GattCommunicationStatus.Success)
        {
            Console.WriteLine($"WriteValueAsync failed. Status={writeStatus}");
        }
    }
}
