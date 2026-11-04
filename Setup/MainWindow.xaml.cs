using System;
using System.Diagnostics;
using System.IO;
using System.IO.Compression;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Interop;
using Microsoft.Win32;

namespace Setup
{
    public partial class MainWindow : Window
    {
        [DllImport("dwmapi.dll")]
        private static extern int DwmSetWindowAttribute(IntPtr hwnd, int attr, ref int attrValue, int attrSize);

        private string detectedGameDir = @"C:\Program Files (x86)\Steam\steamapps\common\Kingdom Two Crowns";
        private string targetInstallDir = "";
        private bool isFinished = false;

        public MainWindow()
        {
            InitializeComponent();
            Loaded += MainWindow_Loaded;
        }

        private void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
            ApplyDarkTitleBar();
            DetectGame();

            string localAppData = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);
            targetInstallDir = Path.Combine(localAppData, "Programs", "KingdomTwoCrownsTrainer");
            txtInstallPath.Text = targetInstallDir;
        }

        private void ApplyDarkTitleBar()
        {
            try
            {
                var helper = new WindowInteropHelper(this);
                int dark = 1;
                DwmSetWindowAttribute(helper.Handle, 20, ref dark, sizeof(int));
                DwmSetWindowAttribute(helper.Handle, 19, ref dark, sizeof(int));
            }
            catch { }
        }

        private void DetectGame()
        {
            try
            {
                if (Directory.Exists(detectedGameDir))
                {
                    txtGamePath.Text = detectedGameDir;
                    return;
                }

                // Check Steam path in registry
                using var key = Registry.CurrentUser.OpenSubKey(@"Software\Valve\Steam");
                if (key != null)
                {
                    string? steamPath = key.GetValue("SteamPath") as string;
                    if (!string.IsNullOrEmpty(steamPath))
                    {
                        string candidate = Path.Combine(steamPath.Replace('/', '\\'), @"steamapps\common\Kingdom Two Crowns");
                        if (Directory.Exists(candidate))
                        {
                            detectedGameDir = candidate;
                            txtGamePath.Text = detectedGameDir;
                            return;
                        }
                    }
                }

                txtGamePath.Text = "Папка не найдена автоматически. Будет использован стандартный лаунчер.";
                txtGamePath.Foreground = System.Windows.Media.Brushes.Gray;
                chkAutoIntegrate.IsChecked = false;
                chkAutoIntegrate.IsEnabled = false;
            }
            catch
            {
                txtGamePath.Text = detectedGameDir;
            }
        }

        private void BtnBrowse_Click(object sender, RoutedEventArgs e)
        {
            var dlg = new OpenFolderDialog
            {
                Title = "Выберите папку для установки",
                InitialDirectory = targetInstallDir
            };
            if (dlg.ShowDialog() == true)
            {
                targetInstallDir = dlg.FolderName;
                txtInstallPath.Text = targetInstallDir;
            }
        }

        private void BtnCancel_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }

        private async void BtnAction_Click(object sender, RoutedEventArgs e)
        {
            if (isFinished)
            {
                if (chkLaunchAfter.IsChecked == true)
                {
                    string exePath = Path.Combine(targetInstallDir, "CheatLauncher.exe");
                    if (File.Exists(exePath))
                    {
                        Process.Start(new ProcessStartInfo
                        {
                            FileName = exePath,
                            WorkingDirectory = targetInstallDir,
                            UseShellExecute = true
                        });
                    }
                }
                Close();
                return;
            }

            targetInstallDir = txtInstallPath.Text.Trim();
            if (string.IsNullOrEmpty(targetInstallDir))
            {
                MessageBox.Show("Укажите папку для установки.", "Ошибка", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            pnlOptions.Visibility = Visibility.Collapsed;
            pnlProgress.Visibility = Visibility.Visible;
            btnAction.IsEnabled = false;
            btnCancel.IsEnabled = false;

            await Task.Run(() => PerformInstallation());

            pnlProgress.Visibility = Visibility.Collapsed;
            pnlFinished.Visibility = Visibility.Visible;
            btnAction.Content = "Готово";
            btnAction.IsEnabled = true;
            btnCancel.Visibility = Visibility.Collapsed;
            isFinished = true;
        }

        private void PerformInstallation()
        {
            UpdateProgress(10, "Создание каталога установки...");
            Directory.CreateDirectory(targetInstallDir);

            UpdateProgress(25, "Извлечение файлов программы...");
            var asm = Assembly.GetExecutingAssembly();
            using (var stream = asm.GetManifestResourceStream("Setup.Resources.payload.zip"))
            {
                if (stream != null)
                {
                    using var archive = new ZipArchive(stream, ZipArchiveMode.Read);
                    archive.ExtractToDirectory(targetInstallDir, true);
                }
            }

            // Direct Steam Integration
            bool autoIntegrate = false;
            Dispatcher.Invoke(() => autoIntegrate = chkAutoIntegrate.IsChecked == true);
            if (autoIntegrate && Directory.Exists(detectedGameDir))
            {
                UpdateProgress(50, "Настройка интеграции с игрой (version.dll)...");
                try
                {
                    string srcDll = Path.Combine(targetInstallDir, "version.dll");
                    if (File.Exists(srcDll))
                    {
                        File.Copy(srcDll, Path.Combine(detectedGameDir, "version.dll"), true);
                    }
                }
                catch { }
            }

            // Shortcuts
            bool createDesktop = false;
            bool createStartMenu = false;
            Dispatcher.Invoke(() =>
            {
                createDesktop = chkDesktopShortcut.IsChecked == true;
                createStartMenu = chkStartMenuShortcut.IsChecked == true;
            });

            string launcherExe = Path.Combine(targetInstallDir, "CheatLauncher.exe");

            if (createDesktop)
            {
                UpdateProgress(70, "Создание ярлыка на Рабочем столе...");
                string desktopPath = Environment.GetFolderPath(Environment.SpecialFolder.DesktopDirectory);
                string shortcutPath = Path.Combine(desktopPath, "Kingdom Two Crowns Trainer.lnk");
                CreateShortcut(shortcutPath, launcherExe, targetInstallDir);
            }

            if (createStartMenu)
            {
                UpdateProgress(80, "Создание ярлыка в меню «Пуск»...");
                string startMenuPath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.StartMenu), "Programs", "Kingdom Two Crowns Trainer");
                Directory.CreateDirectory(startMenuPath);
                string shortcutPath = Path.Combine(startMenuPath, "Kingdom Two Crowns Trainer.lnk");
                CreateShortcut(shortcutPath, launcherExe, targetInstallDir);
            }

            // Create Uninstaller
            UpdateProgress(90, "Создание модуля удаления...");
            CreateUninstaller(targetInstallDir, detectedGameDir);

            // Register in Windows Programs & Features
            RegisterUninstallEntry(targetInstallDir);

            UpdateProgress(100, "Установка завершена!");
        }

        private void UpdateProgress(double percent, string detail)
        {
            Dispatcher.Invoke(() =>
            {
                pbProgress.Value = percent;
                txtProgressDetail.Text = detail;
            });
            System.Threading.Thread.Sleep(150);
        }

        private static void CreateShortcut(string shortcutPath, string targetPath, string workingDir)
        {
            try
            {
                Type? shellType = Type.GetTypeFromProgID("WScript.Shell");
                if (shellType != null)
                {
                    dynamic shell = Activator.CreateInstance(shellType)!;
                    dynamic shortcut = shell.CreateShortcut(shortcutPath);
                    shortcut.TargetPath = targetPath;
                    shortcut.WorkingDirectory = workingDir;
                    shortcut.Description = "Kingdom: Two Crowns In-Game Trainer & Launcher";
                    shortcut.Save();
                }
            }
            catch { }
        }

        private static void CreateUninstaller(string installDir, string gameDir)
        {
            try
            {
                string uninstallerCmd = Path.Combine(installDir, "uninstall.cmd");
                string content = $@"@echo off
chcp 65001 >nul
echo Удаление Kingdom Two Crowns Trainer...

taskkill /F /IM CheatLauncher.exe >nul 2>&1
if exist ""{gameDir}\version.dll"" del /f /q ""{gameDir}\version.dll"" >nul 2>&1

set DESKTOP=%USERPROFILE%\Desktop\Kingdom Two Crowns Trainer.lnk
if exist ""%DESKTOP%"" del /f /q ""%DESKTOP%"" >nul 2>&1

set STARTMENU=%APPDATA%\Microsoft\Windows\Start Menu\Programs\Kingdom Two Crowns Trainer
if exist ""%STARTMENU%"" rd /s /q ""%STARTMENU%"" >nul 2>&1

reg delete ""HKCU\Software\Microsoft\Windows\CurrentVersion\Uninstall\KingdomTwoCrownsTrainer"" /f >nul 2>&1

echo Очистка файлов...
timeout /t 1 >nul
(goto) 2>nul & rd /s /q ""{installDir}""
";
                File.WriteAllText(uninstallerCmd, content);
            }
            catch { }
        }

        private static void RegisterUninstallEntry(string installDir)
        {
            try
            {
                using var key = Registry.CurrentUser.CreateSubKey(@"Software\Microsoft\Windows\CurrentVersion\Uninstall\KingdomTwoCrownsTrainer");
                if (key != null)
                {
                    key.SetValue("DisplayName", "Kingdom: Two Crowns — In-Game Trainer");
                    key.SetValue("DisplayVersion", "2.5.0");
                    key.SetValue("Publisher", "ficksj");
                    key.SetValue("DisplayIcon", Path.Combine(installDir, "CheatLauncher.exe"));
                    key.SetValue("UninstallString", $"cmd.exe /c \"{Path.Combine(installDir, "uninstall.cmd")}\"");
                    key.SetValue("InstallLocation", installDir);
                }
            }
            catch { }
        }
    }
}
