namespace BLEMIXClient;

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
        commands.AddCommand<ShellCommand>();
    }
}

[Command("shell", "Shell")]
public sealed class ShellCommand : ICommandHandler
{
    private static readonly Guid ServiceUuid = Guid.Parse("6E400001-B5A3-F393-E0A9-E50E24DCCA9E"); // UART Service
    private static readonly Guid RxCharUuid = Guid.Parse("6E400002-B5A3-F393-E0A9-E50E24DCCA9E"); // UART RX (Write)
    private static readonly Guid TxCharUuid = Guid.Parse("6E400003-B5A3-F393-E0A9-E50E24DCCA9E"); // UART TX (Notify)

    private static readonly SemaphoreSlim NotificationSemaphore = new(0);
    private static BluetoothLEDevice? device;
    private static GattDeviceService? service;
    private static GattCharacteristic? rxChar;
    private static GattCharacteristic? txChar;
    private static string lastNotifyData = "";
    private static bool waitingForResponse;

    [Option<string>("--address", "-a", Description = "Address", Required = true)]
    public string Address { get; set; } = default!;

    public async ValueTask ExecuteAsync(CommandContext context)
    {
        Console.WriteLine("BLE ATOM Control Shell");
        Console.WriteLine("======================");
        Console.WriteLine();

        while (true)
        {
            Console.WriteLine();
            Console.WriteLine("Commands:");
            Console.WriteLine("  1. Connect");
            Console.WriteLine("  2. Disconnect");
            Console.WriteLine("  3. Set LED (RED/GREEN/BLUE/WHITE/OFF)");
            Console.WriteLine("  4. Set LED (RGB)");
            Console.WriteLine("  5. Get Temperature");
            Console.WriteLine("  6. Exit");
            Console.Write("Select> ");

            var input = Console.ReadLine()?.Trim();
            if (string.IsNullOrEmpty(input)) continue;

            try
            {
                switch (input)
                {
                    case "1":
                        await ConnectAsync();
                        break;
                    case "2":
                        await DisconnectAsync();
                        break;
                    case "3":
                        await SetLedPresetAsync();
                        break;
                    case "4":
                        await SetLedRgbAsync();
                        break;
                    case "5":
                        await GetTemperatureAsync();
                        break;
                    case "6":
                        await DisconnectAsync();
                        Console.WriteLine("Goodbye!");
                        return;
                    default:
                        Console.WriteLine("Invalid command.");
                        break;
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error: {ex.Message}");
            }
        }
    }

    private async Task ConnectAsync()
    {
        if (device != null)
        {
            Console.WriteLine("Already connected. Disconnect first.");
            return;
        }

        Console.WriteLine("Connecting...");
        var address = Address.Replace(":", string.Empty).Replace("-", string.Empty);
        device = await BluetoothLEDevice.FromBluetoothAddressAsync(Convert.ToUInt64(address, 16));

        if (device == null)
        {
            Console.WriteLine("Bluetooth connect failed.");
            return;
        }

        Console.WriteLine($"Connected to: {device.Name}");

        // Get UART Service
        var servicesResult = await device.GetGattServicesAsync(BluetoothCacheMode.Uncached);
        if (servicesResult.Status != GattCommunicationStatus.Success)
        {
            Console.WriteLine($"Get services failed. Status={servicesResult.Status}");
            await DisconnectAsync();
            return;
        }

        service = servicesResult.Services.FirstOrDefault(s => s.Uuid == ServiceUuid);
        if (service == null)
        {
            Console.WriteLine("UART service not found.");
            await DisconnectAsync();
            return;
        }

        var access = await service.RequestAccessAsync();
        if (access != DeviceAccessStatus.Allowed)
        {
            Console.WriteLine($"Request access failed. Status={access}");
            await DisconnectAsync();
            return;
        }

        // Get Characteristics
        var charsResult = await service.GetCharacteristicsAsync(BluetoothCacheMode.Uncached);
        if (charsResult.Status != GattCommunicationStatus.Success)
        {
            Console.WriteLine($"Get characteristics failed. Status={charsResult.Status}");
            await DisconnectAsync();
            return;
        }

        rxChar = charsResult.Characteristics.FirstOrDefault(c => c.Uuid == RxCharUuid);
        txChar = charsResult.Characteristics.FirstOrDefault(c => c.Uuid == TxCharUuid);

        if (rxChar == null)
        {
            Console.WriteLine("RX characteristic not found.");
            await DisconnectAsync();
            return;
        }

        if (txChar == null)
        {
            Console.WriteLine("TX characteristic not found.");
            await DisconnectAsync();
            return;
        }

        // Subscribe to notifications
        var cccdResult = await txChar.WriteClientCharacteristicConfigurationDescriptorAsync(
            GattClientCharacteristicConfigurationDescriptorValue.Notify);

        if (cccdResult != GattCommunicationStatus.Success)
        {
            Console.WriteLine($"Failed to enable notifications. Status={cccdResult}");
        }
        else
        {
            txChar.ValueChanged += TxChar_ValueChanged;
            Console.WriteLine("Notifications enabled.");
        }

        Console.WriteLine("Connection established successfully!");
    }

    private static void TxChar_ValueChanged(GattCharacteristic sender, GattValueChangedEventArgs args)
    {
        var reader = DataReader.FromBuffer(args.CharacteristicValue);
        var bytes = new byte[reader.UnconsumedBufferLength];
        reader.ReadBytes(bytes);
        lastNotifyData = Encoding.ASCII.GetString(bytes);

        // 応答待ちの場合はセマフォを解放するだけ（表示は呼び出し元で行う）
        if (waitingForResponse)
        {
            if (NotificationSemaphore.CurrentCount == 0)
            {
                NotificationSemaphore.Release();
            }
        }
        else
        {
            // 非同期通知の場合のみここで表示
            Console.WriteLine($"\n[Notification] {lastNotifyData.TrimEnd()}");
            Console.Write("Select> ");
        }
    }

    private static async Task DisconnectAsync()
    {
        if (txChar != null)
        {
            txChar.ValueChanged -= TxChar_ValueChanged;
            txChar = null;
        }

        rxChar = null;

        if (service != null)
        {
            service.Dispose();
            service = null;
        }

        if (device != null)
        {
            device.Dispose();
            device = null;
            Console.WriteLine("Disconnected.");
        }
        else
        {
            Console.WriteLine("Not connected.");
        }

        await Task.CompletedTask;
    }

    private static async Task EnsureConnectedAsync()
    {
        if (device == null || rxChar == null)
        {
            Console.WriteLine("Not connected. Please connect first (command 1).");
            throw new InvalidOperationException("Not connected");
        }
        await Task.CompletedTask;
    }

    private static async Task SendCommandAsync(string command)
    {
        await EnsureConnectedAsync();

        var cmd = command.EndsWith("\n") ? command : command + "\n";
        var bytes = Encoding.ASCII.GetBytes(cmd);

        using var writer = new DataWriter();
        writer.WriteBytes(bytes);
        var buffer = writer.DetachBuffer();

        var writeStatus = await rxChar!.WriteValueAsync(buffer, GattWriteOption.WriteWithoutResponse);

        if (writeStatus != GattCommunicationStatus.Success)
        {
            Console.WriteLine($"WriteValueAsync failed. Status={writeStatus}");
        }
        else
        {
            Console.WriteLine($"Command sent: {cmd.TrimEnd()}");
        }
    }

    private static async Task SetLedPresetAsync()
    {
        Console.WriteLine("Available colors: RED, GREEN, BLUE, WHITE, OFF");
        Console.Write("Enter color: ");
        var color = Console.ReadLine()?.Trim().ToUpper();

        if (string.IsNullOrEmpty(color))
        {
            Console.WriteLine("Invalid input.");
            return;
        }

        if (color != "RED" && color != "GREEN" && color != "BLUE" &&
            color != "WHITE" && color != "OFF")
        {
            Console.WriteLine("Invalid color.");
            return;
        }

        await SendCommandAsync(color);
    }

    private static async Task SetLedRgbAsync()
    {
        Console.Write("Enter R (0-255): ");
        if (!int.TryParse(Console.ReadLine(), out int r) || r < 0 || r > 255)
        {
            Console.WriteLine("Invalid R value.");
            return;
        }

        Console.Write("Enter G (0-255): ");
        if (!int.TryParse(Console.ReadLine(), out int g) || g < 0 || g > 255)
        {
            Console.WriteLine("Invalid G value.");
            return;
        }

        Console.Write("Enter B (0-255): ");
        if (!int.TryParse(Console.ReadLine(), out int b) || b < 0 || b > 255)
        {
            Console.WriteLine("Invalid B value.");
            return;
        }

        await SendCommandAsync($"RGB {r} {g} {b}");
    }

    private static async Task GetTemperatureAsync()
    {
        await EnsureConnectedAsync();

        // セマフォをリセット
        while (NotificationSemaphore.CurrentCount > 0)
        {
            await NotificationSemaphore.WaitAsync(0);
        }

        lastNotifyData = "";
        waitingForResponse = true;

        await SendCommandAsync("TEMP");

        Console.WriteLine("Waiting for response...");

        // 最大5秒待つ
        var timeoutTask = Task.Delay(5000);
        var notificationTask = NotificationSemaphore.WaitAsync();

        var completedTask = await Task.WhenAny(notificationTask, timeoutTask);

        waitingForResponse = false;

        if (completedTask == notificationTask)
        {
            // Notificationを受信した
            Console.WriteLine($"Result: {lastNotifyData.TrimEnd()}");
        }
        else
        {
            // タイムアウト
            Console.WriteLine("Timeout: No response received. Check connection.");
        }
    }
}