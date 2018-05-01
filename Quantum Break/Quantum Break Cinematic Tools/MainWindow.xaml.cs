using System;
using System.ComponentModel;
using System;
using System.IO;
using System.Windows;
using System.Windows.Input;
using System.ComponentModel;

namespace Quantum_Break_Cinematic_Tools
{
  /// <summary>
  /// Interaction logic for MainWindow.xaml
  /// </summary>
  public partial class MainWindow : Window
  {
    private static String[,] g_injectMap = new String[,] { { "QuantumBreak", "CT_QuantumBreak.dll" } };

    private static String g_shortName = "QuantumBreak";

    private BackgroundWorker bgWorker = new BackgroundWorker();

    private bool isInjected = false;

    public MainWindow()
    {
      InitializeComponent();

      bgWorker.DoWork += BgWorker_DoWork;
      bgWorker.RunWorkerAsync();
    }

    private void BgWorker_DoWork(object sender, DoWorkEventArgs e)
    {
      System.Threading.Thread.Sleep(1000);

      // Check for updated Updater
      if (File.Exists("UpdaterNew.exe"))
      {
        File.Delete("Updater.exe");
        File.Move("UpdaterNew.exe", "Updater.exe");
      }

      if (!File.Exists(g_injectMap[0, 1]))
      {
        MessageBox.Show(g_injectMap[0, 1] + " could not be found!\n" +
          "Please make sure you have exported all the files in the same folder.",
          "Error", MessageBoxButton.OK, MessageBoxImage.Error);
        Shutdown();
      }

      CheckUpdates();
      Inject();

      if (!isInjected)
      {
        MessageBox.Show(g_injectMap[0, 1] + " could not be injected!\n" +
          "Make sure your anti-virus software isn't blocking the tools and make an exception if necessary.\n" +
          "You can also try running the program with administrator privileges", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
      }

      Shutdown();
    }

    private void SetStatus(string status)
    {
      this.Dispatcher.BeginInvoke((Action)delegate ()
      {
        statusLabel.Text = status;
      });
    }

    private void Shutdown()
    {
      Dispatcher.BeginInvoke((Action)delegate ()
      {
        Application.Current.Shutdown();
      });
    }

    private void Window_MouseDown(object sender, MouseButtonEventArgs e)
    {
      if (e.ChangedButton == MouseButton.Left)
        this.DragMove();
    }
  }
}
