using System;
using System.Diagnostics;
using System.IO;
using System.IO.Pipes;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Threading;

namespace WpfLauncher
{
    public partial class MainWindow : Window
    {
        private const string GameProcessName = "KingdomTwoCrowns";
        private const string SteamAppUri = "steam://run/701160";
        private const string GameDirDefault = @"C:\Program Files (x86)\Steam\steamapps\common\Kingdom Two Crowns";

        #region Native Windows API
        [DllImport("dwmapi.dll")]
        private static extern int DwmSetWindowAttribute(IntPtr hwnd, int attr, ref int attrValue, int attrSize);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr OpenProcess(uint dwDesiredAccess, bool bInheritHandle, int dwProcessId);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr VirtualAllocEx(IntPtr hProcess, IntPtr lpAddress, uint dwSize, uint flAllocationType, uint flProtect);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool WriteProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, byte[] lpBuffer, uint nSize, out IntPtr lpNumberOfBytesWritten);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr CreateRemoteThread(IntPtr hProcess, IntPtr lpThreadAttributes, uint dwStackSize, IntPtr lpStartAddress, IntPtr lpParameter, uint dwCreationFlags, out IntPtr lpThreadId);

        [DllImport("kernel32.dll", CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
        private static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

        [DllImport("kernel32.dll", CharSet = CharSet.Auto)]
        private static extern IntPtr GetModuleHandle(string lpModuleName);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool CloseHandle(IntPtr hObject);

        private const uint PROCESS_ALL_ACCESS = 0x1F0FFF;
        private const uint MEM_COMMIT = 0x00001000;
        private const uint MEM_RESERVE = 0x00002000;
        private const uint PAGE_READWRITE = 0x04;
        #endregion

        private DispatcherTimer timerMonitor = null!;
        private bool isInjected = false;
        private int lastProcessId = -1;
        private bool isLogVisible = false;

        public MainWindow()
        {
            InitializeComponent();
            Loaded += MainWindow_Loaded;
            Closing += MainWindow_Closing;
            SetupTimer();
        }

        private void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
            ApplyNativeDarkTitleBar();
            Log("Трейнер готов. Нажмите [Запустить игру] или запустите её через Steam.");
        }

        private void ApplyNativeDarkTitleBar()
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

        private void SetupTimer()
        {
            timerMonitor = new DispatcherTimer
            {
                Interval = TimeSpan.FromSeconds(1)
            };
            timerMonitor.Tick += TimerMonitor_Tick;
            timerMonitor.Start();
        }

        private void TimerMonitor_Tick(object? sender, EventArgs e)
        {
            try
            {
                var procs = Process.GetProcessesByName(GameProcessName);
                if (procs.Length > 0)
                {
                    var proc = procs[0];
                    if (!isInjected || lastProcessId != proc.Id)
                    {
                        lastProcessId = proc.Id;
                        Log($"[ОБНАРУЖЕНИЕ] Процесс найден: {GameProcessName}.exe (PID: {proc.Id})");
                        Task.Run(() => InjectCheat(proc));
                    }
                }
                else
                {
                    if (isInjected)
                    {
                        isInjected = false;
                        lastProcessId = -1;
                        UpdateStatus(false, "Ожидание запуска KingdomTwoCrowns.exe...");
                        Log("[ИНФО] Игра закрыта.");
                        CleanupGameFolder();
                    }
                }
            }
            catch (Exception ex)
            {
                Log($"[ОШИБКА] {ex.Message}");
            }
        }

        private void InjectCheat(Process proc)
        {
            try
            {
                string baseDir = AppDomain.CurrentDomain.BaseDirectory;
                string cheatDllPath = Path.Combine(baseDir, "CheatKTC.dll");
                if (!File.Exists(cheatDllPath))
                {
                    cheatDllPath = Path.GetFullPath(Path.Combine(baseDir, @"..\..\..\..\CheatKTC.dll"));
                }
                if (!File.Exists(cheatDllPath))
                {
                    cheatDllPath = @"c:\Users\kbeletskiy\Documents\CheatKTC\CheatKTC.dll";
                }

                if (!File.Exists(cheatDllPath))
                {
                    Dispatcher.Invoke(() => Log($"[ОШИБКА] Файл чита не найден: {cheatDllPath}"));
                    return;
                }

                IntPtr hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, proc.Id);
                if (hProcess != IntPtr.Zero)
                {
                    byte[] bytes = Encoding.Unicode.GetBytes(cheatDllPath + "\0");
                    IntPtr pAddr = VirtualAllocEx(hProcess, IntPtr.Zero, (uint)bytes.Length, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
                    if (pAddr != IntPtr.Zero)
                    {
                        WriteProcessMemory(hProcess, pAddr, bytes, (uint)bytes.Length, out _);
                        IntPtr hKernel32 = GetModuleHandle("kernel32.dll");
                        IntPtr pLoadLibraryW = GetProcAddress(hKernel32, "LoadLibraryW");
                        CreateRemoteThread(hProcess, IntPtr.Zero, 0, pLoadLibraryW, pAddr, 0, out _);
                    }
                    CloseHandle(hProcess);
                }

                isInjected = true;
                Dispatcher.Invoke(() =>
                {
                    UpdateStatus(true, $"Подключено к игре (PID: {proc.Id}) • Нажмите F1 в игре");
                    Log("[УСПЕХ] Чит подключен! В игре нажмите F1 для вызова экранного меню.");
                });
            }
            catch (Exception ex)
            {
                Dispatcher.Invoke(() => Log($"[ОШИБКА ВНЕДРЕНИЯ] {ex.Message}"));
            }
        }

        private void BtnLaunchGame_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                DeployVersionDll();
                Log("[ЗАПУСК] Запуск Kingdom Two Crowns через Steam...");
                Process.Start(new ProcessStartInfo
                {
                    FileName = SteamAppUri,
                    UseShellExecute = true
                });
            }
            catch (Exception ex)
            {
                Log($"[ОШИБКА ЗАПУСКА] {ex.Message}");
            }
        }

        private void BtnCheat_Click(object sender, RoutedEventArgs e)
        {
            if (sender is Button btn && btn.Tag is string cmd)
            {
                if (SendPipeCommand(cmd))
                {
                    Log($"[КОМАНДА] Выполнено: {btn.Content} ({cmd})");
                }
                else
                {
                    Log("[ИНФО] Игра еще не запущена. Запустите игру через кнопку вверху.");
                }
            }
        }

        private void ToggleCheat_Click(object sender, RoutedEventArgs e)
        {
            if (sender is CheckBox chk && chk.Tag is string cmd)
            {
                if (SendPipeCommand(cmd))
                {
                    Log($"[ПЕРЕКЛЮЧЕНИЕ] {cmd}: {(chk.IsChecked == true ? "ВКЛЮЧЕНО" : "ВЫКЛЮЧЕНО")}");
                }
                else
                {
                    Log("[ИНФО] Игра еще не запущена. Запустите игру через кнопку вверху.");
                    chk.IsChecked = !chk.IsChecked; // revert toggle
                }
            }
        }

        private void BtnLogToggle_Click(object sender, RoutedEventArgs e)
        {
            isLogVisible = !isLogVisible;
            bdrLogDrawer.Visibility = isLogVisible ? Visibility.Visible : Visibility.Collapsed;
            btnLogToggle.Content = isLogVisible ? "Журнал событий ▲" : "Журнал событий ▼";
        }

        private void UpdateStatus(bool connected, string text)
        {
            txtStatus.Text = text;
            if (connected)
            {
                dotStatus.Fill = new SolidColorBrush(Color.FromRgb(16, 185, 129)); // Emerald
                txtStatus.Foreground = new SolidColorBrush(Color.FromRgb(52, 211, 153));
            }
            else
            {
                dotStatus.Fill = new SolidColorBrush(Color.FromRgb(113, 113, 122)); // Gray
                txtStatus.Foreground = new SolidColorBrush(Color.FromRgb(161, 161, 170));
            }
        }

        private static bool SendPipeCommand(string command)
        {
            try
            {
                using var pipe = new NamedPipeClientStream(".", "CheatKTCPipe", PipeDirection.Out);
                pipe.Connect(300);
                using var writer = new StreamWriter(pipe, Encoding.ASCII);
                writer.Write(command);
                writer.Flush();
                return true;
            }
            catch
            {
                return false;
            }
        }

        private void DeployVersionDll()
        {
            try
            {
                if (Directory.Exists(GameDirDefault))
                {
                    string baseDir = AppDomain.CurrentDomain.BaseDirectory;
                    string srcDll = Path.Combine(baseDir, "CheatKTC.dll");
                    if (!File.Exists(srcDll)) srcDll = @"c:\Users\kbeletskiy\Documents\CheatKTC\CheatKTC.dll";

                    if (File.Exists(srcDll))
                    {
                        File.Copy(srcDll, Path.Combine(GameDirDefault, "version.dll"), true);
                    }
                }
            }
            catch { }
        }

        private void CleanupGameFolder()
        {
            try
            {
                string target = Path.Combine(GameDirDefault, "version.dll");
                if (File.Exists(target))
                {
                    File.Delete(target);
                }
            }
            catch { }
        }

        private void Log(string message)
        {
            string time = DateTime.Now.ToString("HH:mm:ss");
            string entry = $"[{time}] {message}";
            lstLog.Items.Add(entry);
            lstLog.ScrollIntoView(entry);
        }

        private void MainWindow_Closing(object? sender, System.ComponentModel.CancelEventArgs e)
        {
            CleanupGameFolder();
        }
    }
}
