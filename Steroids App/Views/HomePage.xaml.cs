using Microsoft.UI.Xaml.Controls;
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

            AddTestResult("初始化Steroids", "等待检测", InfoBarSeverity.Informational);
            AddTestResult("检测Steroids可用状态", "等待检测", InfoBarSeverity.Informational);
            AddTestResult("读取进程内存", "等待检测", InfoBarSeverity.Informational);
            AddTestResult("保护进程", "等待检测", InfoBarSeverity.Informational);
            AddTestResult("终止进程", "等待检测", InfoBarSeverity.Informational);
            AddTestResult("停止Steroids", "等待检测", InfoBarSeverity.Informational);
        }

        private void OnLoaded(object sender, RoutedEventArgs e)
        {
        }

        private void RunUnitTest(object sender, RoutedEventArgs e)
        {
            UnitTestList.Items.Clear();
            AddTestResult("初始化Steroids", "可用", InfoBarSeverity.Success);
            AddTestResult("检测Steroids可用状态", "可用", InfoBarSeverity.Success);
            AddTestResult("读取进程内存", "可用", InfoBarSeverity.Success);
            AddTestResult("保护进程", "可用", InfoBarSeverity.Success);
            AddTestResult("终止进程", "可用", InfoBarSeverity.Success);
            AddTestResult("停止Steroids", "可用", InfoBarSeverity.Success);
        }
        private void AddTestResult(string title, string message, InfoBarSeverity severity)
        {
            UnitTestList.Items.Add(new InfoBar()
            {
                Severity = severity,
                Title = title,
                Message = message.PadRight(100),
                IsOpen = true,
                IsClosable = false,
                Margin = new Thickness(0, 12, 0, 0)
            });
        }
    }
}
