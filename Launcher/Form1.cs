using System;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.IO;
using System.IO.Pipes;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace CheatLauncher
{
    public class Form1 : Form
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

        // Clean Modern Dark Palette
        private static readonly Color ColorBg = Color.FromArgb(18, 18, 20);
        private static readonly Color ColorSurface = Color.FromArgb(24, 24, 28);
        private static readonly Color ColorRowHover = Color.FromArgb(32, 32, 38);
        private static readonly Color ColorBorder = Color.FromArgb(42, 42, 48);
        private static readonly Color ColorTextPrimary = Color.FromArgb(240, 240, 242);
        private static readonly Color ColorTextMuted = Color.FromArgb(136, 136, 144);
        private static readonly Color ColorBadgeBg = Color.FromArgb(34, 34, 40);
        private static readonly Color ColorBadgeBorder = Color.FromArgb(56, 56, 66);
        private static readonly Color ColorGreen = Color.FromArgb(46, 204, 113);
        private static readonly Color ColorGreenMuted = Color.FromArgb(39, 174, 96);
        private static readonly Color ColorBlue = Color.FromArgb(52, 152, 219);

        // UI Controls
        private Panel pnlHeader = null!;
        private Label lblGameTitle = null!;
        private Label lblGameSub = null!;
        private Button btnLaunch = null!;
        private Panel pnlCheatsList = null!;
        private Panel pnlFooter = null!;
        private Label lblStatusDot = null!;
        private Label lblStatusText = null!;
        private Label lblLogToggle = null!;
        private Panel pnlLogDrawer = null!;
        private ListBox lstLog = null!;
        private System.Windows.Forms.Timer timerMonitor = null!;

        private bool isInjected = false;
        private int lastProcessId = -1;
        private bool isLogVisible = false;

        public Form1()
        {
            this.SetStyle(ControlStyles.ResizeRedraw | ControlStyles.OptimizedDoubleBuffer | ControlStyles.AllPaintingInWmPaint, true);
            ApplyNativeDarkTitleBar();
            BuildUI();
            SetupTimer();
            Log("Инициализация завершена. Ожидание запуска игры...");
        }

        private void ApplyNativeDarkTitleBar()
        {
            try
            {
                int dark = 1;
                DwmSetWindowAttribute(this.Handle, 20, ref dark, sizeof(int));
                DwmSetWindowAttribute(this.Handle, 19, ref dark, sizeof(int));
            }
            catch { }
        }

        protected override void OnHandleCreated(EventArgs e)
        {
            base.OnHandleCreated(e);
            ApplyNativeDarkTitleBar();
        }

        private void BuildUI()
        {
            this.Text = "Kingdom: Two Crowns — Trainer";
            this.Size = new Size(680, 720);
            this.MinimumSize = new Size(600, 580);
            this.StartPosition = FormStartPosition.CenterScreen;
            this.BackColor = ColorBg;
            this.ForeColor = ColorTextPrimary;
            this.Font = new Font("Segoe UI", 9.25f, FontStyle.Regular);

            // --- 1. HEADER PANEL ---
            pnlHeader = new Panel
            {
                Dock = DockStyle.Top,
                Height = 72,
                BackColor = ColorSurface,
                Padding = new Padding(24, 0, 24, 0)
            };
            pnlHeader.Paint += (s, e) =>
            {
                using var p = new Pen(ColorBorder, 1);
                e.Graphics.DrawLine(p, 0, pnlHeader.Height - 1, pnlHeader.Width, pnlHeader.Height - 1);
            };

            lblGameTitle = new Label
            {
                Text = "Kingdom: Two Crowns",
                Font = new Font("Segoe UI Semibold", 13.5f, FontStyle.Bold),
                ForeColor = ColorTextPrimary,
                AutoSize = true,
                Location = new Point(24, 14)
            };

            lblGameSub = new Label
            {
                Text = "Внутриигровой трейнер • Steam Edition • Unity IL2CPP",
                Font = new Font("Segoe UI", 8.5f),
                ForeColor = ColorTextMuted,
                AutoSize = true,
                Location = new Point(25, 41)
            };

            btnLaunch = new Button
            {
                Text = "Запустить игру",
                Anchor = AnchorStyles.Top | AnchorStyles.Right,
                Location = new Point(pnlHeader.Width - 170, 18),
                Size = new Size(146, 36),
                BackColor = Color.FromArgb(36, 36, 44),
                ForeColor = ColorTextPrimary,
                Font = new Font("Segoe UI Semibold", 9.5f, FontStyle.Bold),
                FlatStyle = FlatStyle.Flat,
                Cursor = Cursors.Hand
            };
            btnLaunch.FlatAppearance.BorderColor = ColorBorder;
            btnLaunch.FlatAppearance.BorderSize = 1;
            btnLaunch.Click += (s, e) => LaunchGame();
            btnLaunch.MouseEnter += (s, e) =>
            {
                btnLaunch.BackColor = Color.FromArgb(48, 48, 58);
                btnLaunch.FlatAppearance.BorderColor = Color.FromArgb(80, 80, 96);
            };
            btnLaunch.MouseLeave += (s, e) =>
            {
                btnLaunch.BackColor = Color.FromArgb(36, 36, 44);
                btnLaunch.FlatAppearance.BorderColor = ColorBorder;
            };

            pnlHeader.Controls.Add(lblGameTitle);
            pnlHeader.Controls.Add(lblGameSub);
            pnlHeader.Controls.Add(btnLaunch);
            pnlHeader.Resize += (s, e) =>
            {
                btnLaunch.Location = new Point(pnlHeader.Width - 170, 18);
            };
            this.Controls.Add(pnlHeader);

            // --- 2. FOOTER STATUS BAR ---
            pnlFooter = new Panel
            {
                Dock = DockStyle.Bottom,
                Height = 36,
                BackColor = ColorSurface,
                Padding = new Padding(18, 0, 18, 0)
            };
            pnlFooter.Paint += (s, e) =>
            {
                using var p = new Pen(ColorBorder, 1);
                e.Graphics.DrawLine(p, 0, 0, pnlFooter.Width, 0);
            };

            lblStatusDot = new Label
            {
                Text = "●",
                Font = new Font("Segoe UI", 11f, FontStyle.Bold),
                ForeColor = ColorTextMuted,
                AutoSize = true,
                Location = new Point(18, 8)
            };

            lblStatusText = new Label
            {
                Text = "Ожидание запуска игры...",
                Font = new Font("Segoe UI", 9f),
                ForeColor = ColorTextMuted,
                AutoSize = true,
                Location = new Point(36, 9)
            };

            lblLogToggle = new Label
            {
                Text = "Журнал ▼",
                Anchor = AnchorStyles.Top | AnchorStyles.Right,
                Location = new Point(pnlFooter.Width - 90, 9),
                AutoSize = true,
                Font = new Font("Segoe UI", 8.5f),
                ForeColor = ColorTextMuted,
                Cursor = Cursors.Hand
            };
            lblLogToggle.Click += (s, e) => ToggleLogDrawer();
            lblLogToggle.MouseEnter += (s, e) => lblLogToggle.ForeColor = ColorTextPrimary;
            lblLogToggle.MouseLeave += (s, e) => lblLogToggle.ForeColor = ColorTextMuted;

            pnlFooter.Controls.Add(lblStatusDot);
            pnlFooter.Controls.Add(lblStatusText);
            pnlFooter.Controls.Add(lblLogToggle);
            pnlFooter.Resize += (s, e) =>
            {
                lblLogToggle.Location = new Point(pnlFooter.Width - 90, 9);
            };
            this.Controls.Add(pnlFooter);

            // --- 3. LOG DRAWER (Hidden by default) ---
            pnlLogDrawer = new Panel
            {
                Dock = DockStyle.Bottom,
                Height = 130,
                BackColor = Color.FromArgb(14, 14, 16),
                Visible = false,
                Padding = new Padding(12)
            };
            pnlLogDrawer.Paint += (s, e) =>
            {
                using var p = new Pen(ColorBorder, 1);
                e.Graphics.DrawLine(p, 0, 0, pnlLogDrawer.Width, 0);
            };

            lstLog = new ListBox
            {
                Dock = DockStyle.Fill,
                BackColor = Color.FromArgb(14, 14, 16),
                ForeColor = Color.FromArgb(180, 180, 190),
                Font = new Font("Consolas", 8.5f),
                BorderStyle = BorderStyle.None,
                IntegralHeight = false
            };
            pnlLogDrawer.Controls.Add(lstLog);
            this.Controls.Add(pnlLogDrawer);

            // --- 4. CHEATS LIST CONTAINER (Scrollable Clean Rows) ---
            pnlCheatsList = new Panel
            {
                Dock = DockStyle.Fill,
                BackColor = ColorBg,
                AutoScroll = true,
                Padding = new Padding(18, 12, 18, 12)
            };
            this.Controls.Add(pnlCheatsList);

            // Populate Cheats Table
            BuildCheatsList();

            this.FormClosing += Form1_FormClosing;
        }

        private void BuildCheatsList()
        {
            var cheats = new[]
            {
                new CheatItem("Внутриигровое меню чита", "F1", "Открывает полный экранный интерфейс игры", "TOGGLE_MENU", "Открыть"),
                new CheatItem("Бесконечная выносливость", "F4", "Маунт никогда не устает при быстром беге", "INFINITE_STAMINA", "Вкл / Выкл"),
                new CheatItem("Бессмертие правителя", "F5", "Корону невозможно выбить, монарх неуязвим", "INVULNERABLE", "Вкл / Выкл"),
                new CheatItem("Добавить 20 монет", "F2", "Насыпать золотые монеты в кошелек", "GIVE_COINS", "+20 Монет"),
                new CheatItem("Добавить 5 камней", "F3", "Добавить самоцветы (Gems) в суму", "GIVE_GEMS", "+5 Камней"),
                new CheatItem("Мгновенная постройка", "F9", "Моментальное возведение и апгрейд зданий", "AUTO_BUILD", "Вкл / Выкл"),
                new CheatItem("Ускорение игры x2", "F6", "Увеличение скорости движения и игрового темпа", "SPEED", "Ускорить"),
                new CheatItem("Уничтожить всех жадин", "F7", "Мгновенная зачистка всех монстров на острове", "DEFEAT_GREED", "Уничтожить"),
                new CheatItem("Взорвать все порталы", "F8", "Разрушить вражеские порталы жадин", "DESTROY_PORTALS", "Взорвать"),
                new CheatItem("Пропустить текущий день", "F10", "Перемотать время сразу на следующее утро", "SKIP_DAY", "Пропустить"),
                new CheatItem("Заморозка времени суток", "F11", "Остановить смену дня и ночи", "DAY_NIGHT_PAUSE", "Вкл / Выкл"),
                new CheatItem("Вызов Кровавой Луны", "F12", "Принудительный запуск или отмена Blood Moon", "BLOOD_MOON", "Переключить")
            };

            // Table of rows
            TableLayoutPanel tbl = new TableLayoutPanel
            {
                Dock = DockStyle.Top,
                ColumnCount = 1,
                RowCount = cheats.Length,
                AutoSize = true,
                AutoSizeMode = AutoSizeMode.GrowAndShrink,
                BackColor = Color.Transparent
            };
            tbl.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100f));

            for (int i = 0; i < cheats.Length; i++)
            {
                var row = CreateCheatRow(cheats[i], i);
                tbl.Controls.Add(row, 0, i);
            }

            pnlCheatsList.Controls.Add(tbl);
        }

        private Control CreateCheatRow(CheatItem item, int index)
        {
            Panel row = new Panel
            {
                Dock = DockStyle.Top,
                Height = 52,
                BackColor = (index % 2 == 0) ? Color.FromArgb(20, 20, 24) : Color.FromArgb(23, 23, 27),
                Margin = new Padding(0, 1, 0, 1),
                Padding = new Padding(14, 0, 14, 0)
            };

            // Hotkey Badge
            Label lblKey = new Label
            {
                Text = item.Key,
                Size = new Size(46, 26),
                Location = new Point(14, 13),
                BackColor = ColorBadgeBg,
                ForeColor = ColorTextPrimary,
                Font = new Font("Consolas", 9f, FontStyle.Bold),
                TextAlign = ContentAlignment.MiddleCenter
            };
            lblKey.Paint += (s, e) =>
            {
                using var p = new Pen(ColorBadgeBorder, 1);
                e.Graphics.DrawRectangle(p, 0, 0, lblKey.Width - 1, lblKey.Height - 1);
            };

            // Title
            Label lblTitle = new Label
            {
                Text = item.Title,
                Font = new Font("Segoe UI Semibold", 9.5f, FontStyle.Bold),
                ForeColor = ColorTextPrimary,
                AutoSize = true,
                Location = new Point(72, 8)
            };

            // Description
            Label lblDesc = new Label
            {
                Text = item.Description,
                Font = new Font("Segoe UI", 8.25f),
                ForeColor = ColorTextMuted,
                AutoSize = true,
                Location = new Point(73, 28)
            };

            // Action Button
            Button btnAct = new Button
            {
                Text = item.ButtonText,
                Anchor = AnchorStyles.Top | AnchorStyles.Right,
                Location = new Point(row.Width - 130, 11),
                Size = new Size(116, 30),
                BackColor = Color.FromArgb(32, 32, 38),
                ForeColor = ColorTextPrimary,
                Font = new Font("Segoe UI", 8.75f),
                FlatStyle = FlatStyle.Flat,
                Cursor = Cursors.Hand
            };
            btnAct.FlatAppearance.BorderColor = ColorBorder;
            btnAct.FlatAppearance.BorderSize = 1;

            btnAct.Click += (s, e) =>
            {
                if (SendPipeCommand(item.Command))
                {
                    Log($"[КОМАНДА] {item.Title} ({item.Key})");
                }
                else
                {
                    Log($"[ИНФО] Игра еще не запущена. Нажмите [Запустить игру] вверху.");
                }
            };

            btnAct.MouseEnter += (s, e) =>
            {
                btnAct.BackColor = Color.FromArgb(44, 44, 52);
                btnAct.FlatAppearance.BorderColor = Color.FromArgb(70, 70, 84);
            };
            btnAct.MouseLeave += (s, e) =>
            {
                btnAct.BackColor = Color.FromArgb(32, 32, 38);
                btnAct.FlatAppearance.BorderColor = ColorBorder;
            };

            row.Controls.Add(lblKey);
            row.Controls.Add(lblTitle);
            row.Controls.Add(lblDesc);
            row.Controls.Add(btnAct);

            // Responsive positioning
            row.Resize += (s, e) =>
            {
                btnAct.Location = new Point(row.Width - 130, 11);
            };

            // Hover highlighting
            Color origBg = row.BackColor;
            EventHandler enterHandler = (s, e) => row.BackColor = ColorRowHover;
            EventHandler leaveHandler = (s, e) => row.BackColor = origBg;

            row.MouseEnter += enterHandler;
            lblTitle.MouseEnter += enterHandler;
            lblDesc.MouseEnter += enterHandler;
            lblKey.MouseEnter += enterHandler;

            row.MouseLeave += leaveHandler;
            lblTitle.MouseLeave += leaveHandler;
            lblDesc.MouseLeave += leaveHandler;
            lblKey.MouseLeave += leaveHandler;

            return row;
        }

        private void ToggleLogDrawer()
        {
            isLogVisible = !isLogVisible;
            pnlLogDrawer.Visible = isLogVisible;
            lblLogToggle.Text = isLogVisible ? "Журнал ▲" : "Журнал ▼";
        }

        #region Game Injection and Monitoring
        private void SetupTimer()
        {
            timerMonitor = new System.Windows.Forms.Timer { Interval = 1000 };
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
                        UpdateStatus(false, "Ожидание запуска игры...");
                        Log("[ИНФО] Игра была закрыта.");
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
                    Log($"[ОШИБКА] Файл CheatKTC.dll не найден: {cheatDllPath}");
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
                this.Invoke((MethodInvoker)delegate
                {
                    UpdateStatus(true, $"Подключено к игре (PID: {proc.Id}) • Нажмите F1 в игре");
                    Log("[УСПЕХ] Чит-мод активен! В игре нажми F1 для вызова меню.");
                });
            }
            catch (Exception ex)
            {
                Log($"[ОШИБКА ВНЕДРЕНИЯ] {ex.Message}");
            }
        }

        private void LaunchGame()
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

        private void UpdateStatus(bool connected, string text)
        {
            lblStatusText.Text = text;
            if (connected)
            {
                lblStatusDot.ForeColor = ColorGreen;
                lblStatusText.ForeColor = ColorGreen;
            }
            else
            {
                lblStatusDot.ForeColor = ColorTextMuted;
                lblStatusText.ForeColor = ColorTextMuted;
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

        private void Log(string message)
        {
            string time = DateTime.Now.ToString("HH:mm:ss");
            string entry = $"[{time}] {message}";
            if (lstLog.InvokeRequired)
            {
                lstLog.Invoke((MethodInvoker)delegate
                {
                    lstLog.Items.Add(entry);
                    lstLog.TopIndex = lstLog.Items.Count - 1;
                });
            }
            else
            {
                lstLog.Items.Add(entry);
                lstLog.TopIndex = lstLog.Items.Count - 1;
            }
        }

        private void Form1_FormClosing(object? sender, FormClosingEventArgs e)
        {
            CleanupGameFolder();
        }
        #endregion

        private record CheatItem(string Title, string Key, string Description, string Command, string ButtonText);
    }
}
