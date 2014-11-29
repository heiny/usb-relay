using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using DeviceDriver;

namespace DeviceTest
{
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        private void DeviceList_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (DevicesComboBox.SelectedItem != null)
            {
                InvokeSafeAction(LoadDevice);
            }
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            InvokeSafeAction(ListDevices);
        }
        
        private void RefreshButton_Click(object sender, RoutedEventArgs e)
        {
            InvokeSafeAction(ListDevices);
        }

        private void AllOff_Click(object sender, RoutedEventArgs e)
        {
            InvokeSafeAction(CloseAllChannels);
            InvokeSafeAction(LoadDevice);
        }

        private void AllOn_Click(object sender, RoutedEventArgs e)
        {
            InvokeSafeAction(OpenAllChannels);
            InvokeSafeAction(LoadDevice);
        }

        private IRelayDeviceInfo GetCurrentDevice()
        {
            return DevicesComboBox.SelectedItem as IRelayDeviceInfo;
        }

        private void InvokeSafeAction(Action action)
        {
            var actionWrapper = new Action(() =>
            {
                try
                {
                    action();
                }
                catch (Exception ex)
                {
                    MessageBox.Show(this, ex.Message, "Unexpected Error", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            });
            Dispatcher.BeginInvoke(actionWrapper);
        }

        private void OpenAllChannels()
        {
            IRelayDeviceInfo device = GetCurrentDevice();
            if (device == null) return;
            using (var relayController = new RelayController())
            {
                relayController.ConnectDevice(device);
                relayController.OpenAllChannels(device);
            }
        }

        private void CloseAllChannels()
        {
            IRelayDeviceInfo device = GetCurrentDevice();
            if (device == null) return;
            using (var relayController = new RelayController())
            {
                relayController.ConnectDevice(device);
                relayController.CloseAllChannels(device);
            }
        }

        private void ListDevices()
        {
            using (var relayController = new RelayController())
            {
                var devices = relayController.ListDevices();
                while (devices.Count == 0)
                {
                    var result = MessageBox.Show(this, "No devices were found. Would you like to try again?",
                        "No Devices Found",
                        MessageBoxButton.YesNo, MessageBoxImage.Asterisk, MessageBoxResult.Yes);
                    if (result.HasFlag(MessageBoxResult.No))
                    {
                        Application.Current.Shutdown();
                        return;
                    }

                    devices = relayController.ListDevices();
                }

                DevicesComboBox.ItemsSource = devices;
                DevicesComboBox.SelectedIndex = 0;
            }
        }

        private void LoadDevice()
        {
            foreach (ChannelInfoControl child in Channels.Children.OfType<ChannelInfoControl>())
            {
                child.ToggleStateClicked -= ChannelControl_ToggleStateClicked;
            }
            Channels.Children.Clear();

            IRelayDeviceInfo device = GetCurrentDevice();
            if (device == null) return;

            using (var relayController = new RelayController())
            {
                relayController.ConnectDevice(device);

                List<IRelayChannelInfo> channels = relayController.ListChannels(device);
                foreach (var channel in channels)
                {
                    var channelControl = new ChannelInfoControl(device, channel);
                    channelControl.ToggleStateClicked += ChannelControl_ToggleStateClicked;
                    Channels.Children.Add(channelControl);
                }
            }
        }

        private void ChannelControl_ToggleStateClicked(object sender, ChannelToggledEventArgs e)
        {
            using (var relayController = new RelayController())
            {
                relayController.ConnectDevice(e.DeviceInfo);

                var control = sender as ChannelInfoControl;
                control.ChannelInfo = relayController.ToggleChannel(e.DeviceInfo, e.ChannelInfo.Index);
            }
        }
    }
}
