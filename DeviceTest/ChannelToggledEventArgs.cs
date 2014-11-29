using System;
using DeviceDriver;

namespace DeviceTest
{
    public class ChannelToggledEventArgs : EventArgs
    {
        public IRelayChannelInfo ChannelInfo { get; set; }
        public IRelayDeviceInfo DeviceInfo { get; set; }
    }
}
