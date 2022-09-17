using System.Linq;

using Windows.UI;
using Windows.UI.ViewManagement;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=234238

namespace SteroidsApp.Views
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class SettingsPage : Page
    {
        public SettingsPage()
        {
            InitializeComponent();
            Loaded += SettingsPageLoaded;
        }

        private void SettingsPageLoaded(object sender, RoutedEventArgs e)
        {
            var currentTheme = Application.Current.RequestedTheme.ToString();
            ThemePanel.Children.Cast<RadioButton>().FirstOrDefault(radioButton => radioButton?.Tag?.ToString() == currentTheme).IsChecked = true;
        }

        private void OnThemeRadioButtonChecked(object sender, RoutedEventArgs e)
        {
            var selectedTheme = ((RadioButton)sender)?.Tag?.ToString();
            if (selectedTheme == "Light")
            {
                ApplicationView.GetForCurrentView().TitleBar.ButtonForegroundColor = Colors.White;
            }
            else if (selectedTheme == "Dark")
            {
                ApplicationView.GetForCurrentView().TitleBar.ButtonForegroundColor = Colors.Black;
            }
        }
    }
}
