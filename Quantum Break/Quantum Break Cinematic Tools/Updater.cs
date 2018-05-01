using System;
using System.IO;
using System.Net;
using System.Runtime.InteropServices;
using System.Windows;

namespace Quantum_Break_Cinematic_Tools
{
  //https://social.msdn.microsoft.com/Forums/vstudio/en-US/9bdf0eb7-a003-4880-a441-4ce06cf80cbf/whats-the-easiest-way-to-parse-windows-pe-files?forum=csharpgeneral
  public class PeFileHeaderReader
  {
    private readonly IMAGE_FILE_HEADER _fileHeader;

    public PeFileHeaderReader(string path)
    {
      Path = path;

      using (var stream = new FileStream(path, System.IO.FileMode.Open, System.IO.FileAccess.Read))
      {
        var reader = new BinaryReader(stream);

        var dosHeader = FromBinaryReader<IMAGE_DOS_HEADER>(reader);

        reader.BaseStream.Position = stream.Seek(dosHeader.e_lfanew, SeekOrigin.Begin) + 4;

        _fileHeader = FromBinaryReader<IMAGE_FILE_HEADER>(reader);
      }
    }

    public string Path { get; private set; }

    public IMAGE_FILE_HEADER FileHeader
    {
      get
      {
        return _fileHeader;
      }
    }

    public static T FromBinaryReader<T>(BinaryReader reader) where T : struct
    {
      byte[] bytes = reader.ReadBytes(Marshal.SizeOf(typeof(T)));

      GCHandle handle = GCHandle.Alloc(bytes, GCHandleType.Pinned);
      var theStructure = (T)Marshal.PtrToStructure(handle.AddrOfPinnedObject(), typeof(T));
      handle.Free();

      return theStructure;
    }

    public string GetTimeDateStampAsHexString()
    {
      return FileHeader.TimeDateStamp.ToString("X");
    }

    #region File Header Structures

    public struct IMAGE_DOS_HEADER
    {
      public UInt16 e_magic;       // Magic number
      public UInt16 e_cblp;        // Bytes on last page of file
      public UInt16 e_cp;         // Pages in file
      public UInt16 e_crlc;        // Relocations
      public UInt16 e_cparhdr;      // Size of header in paragraphs
      public UInt16 e_minalloc;      // Minimum extra paragraphs needed
      public UInt16 e_maxalloc;      // Maximum extra paragraphs needed
      public UInt16 e_ss;         // Initial (relative) SS value
      public UInt16 e_sp;         // Initial SP value
      public UInt16 e_csum;        // Checksum
      public UInt16 e_ip;         // Initial IP value
      public UInt16 e_cs;         // Initial (relative) CS value
      public UInt16 e_lfarlc;       // File address of relocation table
      public UInt16 e_ovno;        // Overlay number
      public UInt16 e_res_0;       // Reserved words
      public UInt16 e_res_1;       // Reserved words
      public UInt16 e_res_2;       // Reserved words
      public UInt16 e_res_3;       // Reserved words
      public UInt16 e_oemid;       // OEM identifier (for e_oeminfo)
      public UInt16 e_oeminfo;      // OEM information; e_oemid specific
      public UInt16 e_res2_0;       // Reserved words
      public UInt16 e_res2_1;       // Reserved words
      public UInt16 e_res2_2;       // Reserved words
      public UInt16 e_res2_3;       // Reserved words
      public UInt16 e_res2_4;       // Reserved words
      public UInt16 e_res2_5;       // Reserved words
      public UInt16 e_res2_6;       // Reserved words
      public UInt16 e_res2_7;       // Reserved words
      public UInt16 e_res2_8;       // Reserved words
      public UInt16 e_res2_9;       // Reserved words
      public UInt32 e_lfanew;       // File address of new exe header
    }

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct IMAGE_FILE_HEADER
    {
      public UInt16 Machine;
      public UInt16 NumberOfSections;
      public UInt32 TimeDateStamp;
      public UInt32 PointerToSymbolTable;
      public UInt32 NumberOfSymbols;
      public UInt16 SizeOfOptionalHeader;
      public UInt16 Characteristics;
    }

    #endregion File Header Structures
  }

  public partial class MainWindow : Window
  {
    private void CheckUpdates()
    {
      PeFileHeaderReader headerReader = new PeFileHeaderReader(g_injectMap[0, 1]);

      string downloadUrl = "";
      uint localDllVersion = headerReader.FileHeader.TimeDateStamp;
      uint latestDllVersion = 0;

      WebClient webClient = new WebClient();
      StreamReader webStream;

      try
      {
        webStream = new StreamReader(webClient.OpenRead("http://cinetools.xyz/download/version"));
      }
      catch (WebException e)
      {
        MessageBox.Show("Unable to check updates.\n" + e.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
        return;
      }

      while (webStream.Peek() >= 0)
      {
        string line = webStream.ReadLine();
        string[] split = line.Split(' ');
        if (split.Length < 3) continue;
        if (split[0] != g_shortName) continue;

        uint.TryParse(split[1], out latestDllVersion);
        downloadUrl = split[2];

        break;
      }
      webStream.Close();
      webClient.Dispose();

      if (latestDllVersion == 0)
      {
        MessageBox.Show("Unable to check updates.\nCould not find latest version.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
        return;
      }

      if (latestDllVersion > localDllVersion)
      {
        MessageBoxResult result = System.Windows.MessageBox.Show("There is a new version available. Update now?", "Updates available", MessageBoxButton.YesNo, MessageBoxImage.Information);
        if (result == MessageBoxResult.Yes)
        {
          System.Diagnostics.Process.Start("Updater.exe", g_shortName + " " + g_injectMap[0, 1]);
          Shutdown();
        }
      }
    }
  }
}
