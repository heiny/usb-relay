#include "Stdafx.h"
#include "Model.h"
#include "DeviceDriver.h"
#include "RelayController.h"

namespace DeviceDriver {
    RelayController::RelayController(IDeviceDriver^ deviceDriver) {
        m_deviceDriver = deviceDriver;
        m_openDevices = gcnew System::Collections::Generic::Dictionary<String^, USBRelayDeviceInfo^>();
        
        m_deviceDriver->UsbRelayInit();
    }

    RelayController::~RelayController() {
        if (m_isDisposed)
            return;

        this->!RelayController();
        m_isDisposed = true;
    }

    RelayController::!RelayController() {
        for each(String^ key in m_openDevices->Keys) {
            USBRelayDeviceInfo^ deviceInfo = m_openDevices[key];
            if (deviceInfo)  {
                m_deviceDriver->UsbRelayDeviceClose(deviceInfo->m_handle);
            }
        }        
        m_deviceDriver->UsbRelayExit();
    }

    bool RelayController::OpenChannel(IRelayDeviceInfo^ device, ChannelIndex channel) {
        int channelIndex = channel == 0 ? 0 : channel - 1;
        USBRelayDeviceInfo^ realDevice = GetRealDevice(device);
        return m_deviceDriver->UsbRelayDeviceOpenOneRelayChannel(realDevice->m_handle, channelIndex) == 0;
    }

    bool RelayController::CloseChannel(IRelayDeviceInfo^ device, ChannelIndex channel) {
        int channelIndex = channel == 0 ? 0 : channel - 1;
        USBRelayDeviceInfo^ realDevice = GetRealDevice(device);
        return m_deviceDriver->UsbRelayDeviceCloseOneRelayChannel(realDevice->m_handle, channelIndex) == 0;
    }
    
    USBRelayChannelInfo^ RelayController::ToggleChannel(IRelayDeviceInfo^ device, ChannelIndex channel) {
        USBRelayChannelInfo^ currentState = GetChannelInfo(device, channel);
        if (currentState) {
            switch (currentState->State) {
                case ChannelState::Closed:
                    if (OpenChannel(device, channel)) {
                        currentState->State = ChannelState::Opened;
                    }
                    break;
                case ChannelState::Opened:
                    if (CloseChannel(device, channel)) {
                        currentState->State = ChannelState::Closed;
                    }
                    break;
            }
        }
        return currentState;
    }

    bool RelayController::OpenAllChannels(IRelayDeviceInfo^ device) {
        USBRelayDeviceInfo^ realDevice = GetRealDevice(device);
        return m_deviceDriver->UsbRelayDeviceOpenAllRelayChannel(realDevice->m_handle) == 0;
    }

    bool RelayController::CloseAllChannels(IRelayDeviceInfo^ device) {
        USBRelayDeviceInfo^ realDevice = GetRealDevice(device);
        return m_deviceDriver->UsbRelayDeviceCloseAllRelayChannel(realDevice->m_handle) == 0;
    }

    System::Collections::Generic::List<USBRelayChannelInfo^> ^ RelayController::ListChannels(IRelayDeviceInfo^ device) {
        System::Collections::Generic::List<USBRelayChannelInfo^> ^ rv = gcnew System::Collections::Generic::List<USBRelayChannelInfo^>();
        USBRelayDeviceInfo^ realDevice = GetRealDevice(device);

        unsigned int status = 0;
        if (m_deviceDriver->UsbRelayDeviceGetStatus(realDevice->m_handle, &status) == 0) {
            switch (device->Type) {
                case DeviceType::EightChannel:
                    rv->Add(CreateChannelInfo(ChannelIndex::Eight, status));
                    rv->Add(CreateChannelInfo(ChannelIndex::Seven, status));
                    rv->Add(CreateChannelInfo(ChannelIndex::Six, status));
                    rv->Add(CreateChannelInfo(ChannelIndex::Five, status));
                case DeviceType::QuadChannel:
                    rv->Add(CreateChannelInfo(ChannelIndex::Four, status));
                    rv->Add(CreateChannelInfo(ChannelIndex::Three, status));
                case DeviceType::DoubleChannel:
                    rv->Add(CreateChannelInfo(ChannelIndex::Two, status));
                case DeviceType::SingleChannel:
                    rv->Add(CreateChannelInfo(ChannelIndex::One, status));
                break;
            }
        }

        rv->Reverse();
        return rv;
    }

    USBRelayChannelInfo^ RelayController::GetChannelInfo(IRelayDeviceInfo^ device, ChannelIndex channel) {
        unsigned int status = 0;
        USBRelayDeviceInfo^ realDevice = GetRealDevice(device);
        if (m_deviceDriver->UsbRelayDeviceGetStatus(realDevice->m_handle, &status) == 0) {
            return CreateChannelInfo(channel, status);
        }
        return nullptr;
    }
    
    USBRelayChannelInfo^ RelayController::CreateChannelInfo(ChannelIndex channel, unsigned int status) {
        USBRelayChannelInfo^ channelInfo = gcnew USBRelayChannelInfo();
        channelInfo->Index = channel;
        channelInfo->State = GetState(channel, status);
        return channelInfo;
    }

    ChannelState RelayController::GetState(ChannelIndex channel, unsigned int status) {
        int index = channel == 0 ? 0 : channel - 1;
        return (ChannelState)((1 << index) & status);
    }

    USBRelayDeviceInfo^ RelayController::GetRealDevice(IRelayDeviceInfo^ device) {
        if (m_openDevices->ContainsKey(device->SerialNumber)) {
            return m_openDevices[device->SerialNumber];
        }
        
        return nullptr; // USBRelayDeviceInfo^ rv = m_openDevices->
    }

    bool RelayController::ConnectDevice(IRelayDeviceInfo^ device) {
        USBRelayDeviceInfo^ realDevice = GetRealDevice(device);

        if (realDevice->m_handle == 0) {
            IntPtr snPtr = Marshal::StringToHGlobalAnsi(realDevice->SerialNumber);
            int handle = m_deviceDriver->UsbRelayDeviceOpenWithSerialNumber((const char*)snPtr.ToPointer(), realDevice->SerialNumber->Length);
            realDevice->m_handle = handle;
            Marshal::FreeHGlobal(snPtr);

            m_openDevices->Add(realDevice->SerialNumber, realDevice);

            return handle > 0;
        }
        return false;
    }

    System::Collections::Generic::List<IRelayDeviceInfo^> ^ RelayController::ListDevices() {
        System::Collections::Generic::List<IRelayDeviceInfo^> ^ rv = gcnew System::Collections::Generic::List<IRelayDeviceInfo^>();

        usb_relay_device_info * devices = m_deviceDriver->UsbRelayDeviceEnumerate();
        if (devices) {
            struct usb_relay_device_info *currentDevice = devices;
            while (currentDevice) {
                USBRelayDeviceInfo^ deviceInfo = gcnew USBRelayDeviceInfo();
                deviceInfo->SerialNumber = gcnew String(reinterpret_cast<const char*>(currentDevice->serial_number));
                deviceInfo->DevicePath = gcnew String(reinterpret_cast<const char *>(currentDevice->device_path));
                deviceInfo->Type = (DeviceType)currentDevice->type;
                rv->Add(deviceInfo);
                currentDevice = currentDevice->next;
            }
            m_deviceDriver->UsbRelayDeviceFreeEnumerate(devices);
            devices = nullptr;
        }

        return rv;
    }
}