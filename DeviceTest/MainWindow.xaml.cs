using System;
using System.Windows;
using System.Windows.Controls;
using DeviceDriver;

namespace DeviceTest
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private readonly IRelayController _relaydriver;

        public MainWindow()
        {
            InitializeComponent();

            _relaydriver = new RelayController();
        }

        private void DeviceList_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            var comboBox = sender as ComboBox;
            if (comboBox.SelectedItem != null)
            {
                
            }
        }

        private void Window_Closed(object sender, System.EventArgs e)
        {
            _relaydriver.Dispose();
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            Dispatcher.BeginInvoke(new Action(LoadDevices));
        }

        private void LoadDevices()
        {
            var devices = _relaydriver.ListDevices();
            while (devices.Count == 0)
            {
                var result = MessageBox.Show(this, "No devices were found. Would you like to try again?", "No Devices Found",
                    MessageBoxButton.YesNo, MessageBoxImage.Asterisk, MessageBoxResult.Yes);
                if (result.HasFlag(MessageBoxResult.No))
                {
                    Application.Current.Shutdown();
                    return;
                }
            }

            DevicesComboBox.ItemsSource = devices;
            DevicesComboBox.SelectedIndex = 0;
        }
    }
}
