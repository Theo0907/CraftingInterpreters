cd .\Tools\UnitTest
dart pub get
dart bin\test.dart chap24_calls --interpreter ..\..\x64\Release\BytecodeVirtualMachine.exe
pause