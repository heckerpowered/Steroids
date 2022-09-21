using System;

using SteamworksSharp;
using SteamworksSharp.Native;

using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=234238

namespace SteroidsApp.Views
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class HomePage : Page
    {
        public HomePage()
        {
            InitializeComponent();
            Loaded += OnLoaded;
        }

        private void OnLoaded(object sender, RoutedEventArgs e)
        {
            try
            {
                SteamNative.Initialize();
                SteamApi.Initialize(381210);
            }
            catch (Exception exception)
            {
                var dialog = new ContentDialog
                {
                    XamlRoot = XamlRoot,
                    Style = Application.Current.Resources["DefaultContentDialogStyle"] as Style,
                    Title = "Pretty good job so far",
                    CloseButtonText = "Ok",
                    DefaultButton = ContentDialogButton.Primary,
                    Content = exception.ToString()
                };

                _ = dialog.ShowAsync();
            }
        }
    }
}
