package us.xyzw;

public class MainActivity extends android.app.NativeActivity {
  static {
    System.loadLibrary("vrapi");
    System.loadLibrary("vrapp");
  }
}
