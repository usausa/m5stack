using System.Diagnostics;
using System.Text;
using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.Advertisement;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using Windows.Devices.Enumeration;

var serviceUuid = Guid.Parse("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
var characteristicsUuid = Guid.Parse("6E400002-B5A3-F393-E0A9-E50E24DCCA9E");

var connecting = new SemaphoreSlim(1, 1);

var watcher = new BluetoothLEAdvertisementWatcher();

watcher.Received += WatcherOnReceived;

watcher.Start();

Log("1: Start scan.");

Console.ReadLine();

async void WatcherOnReceived(BluetoothLEAdvertisementWatcher sender, BluetoothLEAdvertisementReceivedEventArgs args)
{
    try
    {
        var device = await BluetoothLEDevice.FromBluetoothAddressAsync(args.BluetoothAddress);
        if ((device?.Name is null) || !device.Name.StartsWith("ATOM-UART"))
        {
            return;
        }

        if (!await connecting.WaitAsync(0))
        {
            return;
        }

        Log("2: Device detected.");

        try
        {
            Log("3: Start get service.");

            var services = await device.GetGattServicesAsync(BluetoothCacheMode.Uncached);
            if (services.Status != GattCommunicationStatus.Success)
            {
                Log($"Get service failed. status=[{services.Status}]");
                return;
            }

            var service = services.Services.FirstOrDefault(x => x.Uuid == serviceUuid);
            if (service == null)
            {
                Log("Service not found.");
                return;
            }

            Log("4: Start request access.");

            var accessStatus = await service.RequestAccessAsync();
            if (accessStatus != DeviceAccessStatus.Allowed)
            {
                Log($"request access failed. status=[{accessStatus}]");
                return;
            }

            Log("5: Start get characteristic.");

            var characteristics = await service.GetCharacteristicsAsync(BluetoothCacheMode.Uncached);
            if (characteristics.Status != GattCommunicationStatus.Success)
            {
                Log($"Get characteristics failed. status=[{characteristics.Status}]");
                return;
            }

            var characteristic = characteristics.Characteristics.FirstOrDefault(x => x.Uuid == characteristicsUuid);
            if (characteristic == null)
            {
                Log("Characteristic not found.");
                return;
            }

            Log("6: Start start write.");

            var command = "RGB 0 0 255\n";
            var bytes = Encoding.ASCII.GetBytes(command);
            var writer = new Windows.Storage.Streams.DataWriter();
            writer.WriteBytes(bytes);
            var buffer = writer.DetachBuffer();

            var status = await characteristic.WriteValueAsync(buffer, GattWriteOption.WriteWithoutResponse);
            if (status != GattCommunicationStatus.Success)
            {
                Log($"Write failed. {status}");
                return;
            }

            Log("7: Write completed.");

            watcher.Stop();
        }
        finally
        {
            connecting.Release();
        }
    }
    catch (Exception ex)
    {
        Log($"Unknown exception. {ex.Message}");
    }
}

void Log(string message)
{
    Console.WriteLine($"{DateTime.Now:HH:mm:ss} {message}");
    Debug.WriteLine(message);
}
