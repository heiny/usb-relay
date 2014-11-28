#pragma once

using namespace System;

namespace DeviceDriver {
    public enum DeviceType
    {
        SingleChannel = 1,
        DoubleChannel = 2,
        QuadChannel = 4,
        EightChannel = 8
    };

    public enum ChannelIndex {
        One = 1,
        Two = 2,
        Three = 3,
        Four = 4,
        Five = 5,
        Six = 6,
        Seven = 7,
        Eight = 8
    };

    public enum ChannelState {
        Closed = 0,
        Opened = 1
    };

    public ref class USBRelayChannelInfo {
    public:
        property ChannelIndex Index;
        property ChannelState State;
    };

    public interface class IRelayDeviceInfo {
        property String^ SerialNumber {
            String^ get();
        };
        property String^ DevicePath {
            String^ get();
        };
        property DeviceType Type {
            DeviceType get();
        };
    };

    public interface class IRelayController : public IDisposable {
        System::Collections::Generic::List<IRelayDeviceInfo^> ^ ListDevices();
        System::Collections::Generic::List<USBRelayChannelInfo^> ^ ListChannels(IRelayDeviceInfo^ device);

        USBRelayChannelInfo^ GetChannelInfo(IRelayDeviceInfo^ device, ChannelIndex channel);

        bool ConnectDevice(IRelayDeviceInfo^ device);

        bool OpenChannel(IRelayDeviceInfo^ device, ChannelIndex channel);
        bool CloseChannel(IRelayDeviceInfo^ device, ChannelIndex channel);
        USBRelayChannelInfo^ ToggleChannel(IRelayDeviceInfo^ device, ChannelIndex channel);

        bool OpenAllChannels(IRelayDeviceInfo^ device);
        bool CloseAllChannels(IRelayDeviceInfo^ device);
    };
}