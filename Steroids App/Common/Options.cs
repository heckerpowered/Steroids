using Windows.Storage;
using Windows.UI.Xaml;

namespace SteroidsApp.Common
{
    public static class LocalSettings
    {
        public static ElementTheme RequestedTheme
        {
            get => (ElementTheme)ApplicationData.Current.LocalSettings.Values["RequestedTheme"];
            set => ApplicationData.Current.LocalSettings.Values["RequestedTheme"] = value;
        }
    }
}
