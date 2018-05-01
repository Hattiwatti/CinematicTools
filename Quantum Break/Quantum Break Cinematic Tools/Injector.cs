using System;
using System.Diagnostics;
using System.IO;
using System.Text;
using System.Runtime.InteropServices;
using System.Windows;

namespace Quantum_Break_Cinematic_Tools
{
  public partial class MainWindow : Window
  {
    static readonly IntPtr IntptrZero = (IntPtr)0;

    [DllImport("kernel32.dll", SetLastError = true)]
    static extern IntPtr OpenProcess(uint dwDesiredAccess, int bInheritHandle, uint dwProcessId);

    [DllImport("kernel32.dll", SetLastError = true)]
    static extern IntPtr GetProcAddress(IntPtr hModule, string lpProcName);

    [DllImport("kernel32.dll", SetLastError = true)]
    static extern IntPtr GetModuleHandle(string lpModuleName);

    [DllImport("kernel32.dll", SetLastError = true)]
    static extern IntPtr VirtualAllocEx(IntPtr hProcess, IntPtr lpAddress, IntPtr dwSize, uint flAllocationType, uint flProtect);

    [DllImport("kernel32.dll", SetLastError = true)]
    static extern int WriteProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, byte[] buffer, uint size, int lpNumberOfBytesWritten);

    [DllImport("kernel32.dll", SetLastError = true)]
    static extern IntPtr CreateRemoteThread(IntPtr hProcess, IntPtr lpThreadAttribute, IntPtr dwStackSize, IntPtr lpStartAddress,
        IntPtr lpParameter, uint dwCreationFlags, IntPtr lpThreadId);

    [DllImport("kernel32.dll")]
    static extern int CloseHandle(IntPtr hObject);

    private void Inject()
    {
      uint procId = 0;
      String exePath = "";
      String exeShort = "";
      String dllPath = "";

      Process[] procs = Process.GetProcesses();
      SetStatus("Waiting for the game to launch");

      while (procId == 0)
      {
        for (int i = 0; i < procs.Length; ++i)
        {
          for (int x = 0; x < g_injectMap.GetLength(0); ++x)
          {
            if (procs[i].ProcessName == g_injectMap[x, 0])
            {
              procId = (uint)procs[i].Id;
              dllPath = g_injectMap[x, 1];

              exeShort = procs[i].ProcessName;
              exePath = System.IO.Path.GetDirectoryName(procs[i].MainModule.FileName);
            }
          }
        }

        System.Threading.Thread.Sleep(1000);
      }

      SetStatus("Injecting " + dllPath + " to " + exeShort + ".exe");
      System.Threading.Thread.Sleep(1000);

      String sDllPath = System.IO.Path.GetFullPath(dllPath);
      IntPtr hndProc = OpenProcess((0x2 | 0x8 | 0x10 | 0x20 | 0x400), 1, procId);
      if (hndProc == IntptrZero)
      {
        MessageBox.Show("OpenProcess failed. GetLastError " + Marshal.GetLastWin32Error(), "Error", MessageBoxButton.OK, MessageBoxImage.Error);
        return;
      }

      Process targetProcess = Process.GetProcessById((int)procId);
      foreach (ProcessModule module in targetProcess.Modules)
      {
        if (module.FileName == sDllPath)
        {
          isInjected = true;
          return;
        }
      }

      IntPtr lpLlAddress = GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
      if (lpLlAddress == IntptrZero)
      {
        MessageBox.Show("GetProcAddress failed. GetLastError " + Marshal.GetLastWin32Error(), "Error", MessageBoxButton.OK, MessageBoxImage.Error);
        return;
      }

      IntPtr lpAddress = VirtualAllocEx(hndProc, (IntPtr)null, (IntPtr)sDllPath.Length, (0x1000 | 0x2000), 0X40);
      if (lpAddress == IntptrZero)
      {
        MessageBox.Show("VirtualAllocEx failed. GetLastError " + Marshal.GetLastWin32Error(), "Error", MessageBoxButton.OK, MessageBoxImage.Error);
        return;
      }

      byte[] bytes = Encoding.ASCII.GetBytes(sDllPath);
      if (WriteProcessMemory(hndProc, lpAddress, bytes, (uint)bytes.Length, 0) == 0)
      {
        MessageBox.Show("WriteProcessMemory failed. GetLastError " + Marshal.GetLastWin32Error(), "Error", MessageBoxButton.OK, MessageBoxImage.Error);
        return;
      }

      System.Threading.Thread.Sleep(100);

      if (CreateRemoteThread(hndProc, (IntPtr)null, IntptrZero, lpLlAddress, lpAddress, 0, (IntPtr)null) == IntptrZero)
      {
        MessageBox.Show("CreateRemoteThread failed. GetLastError " + Marshal.GetLastWin32Error(), "Error", MessageBoxButton.OK, MessageBoxImage.Error);
        return;
      }

      CloseHandle(hndProc);
      System.Threading.Thread.Sleep(1000);

      // Iterate modules for the second time to confirm injection
      targetProcess = Process.GetProcessById((int)procId);
      foreach (ProcessModule module in targetProcess.Modules)
      {
        if (module.FileName == sDllPath)
        {
          isInjected = true;
          return;
        }
      }

      isInjected = false;
    }

  }
}
