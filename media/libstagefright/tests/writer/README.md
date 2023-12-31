## Media Testing ##
---
#### Writer :
The Writer Test Suite validates the writers available in libstagefright.

Run the following steps to build the test suite:
```
mmm frameworks/av/media/libstagefright/tests/writer/
```

The 32-bit binaries will be created in the following path : ${OUT}/data/nativetest/
The 64-bit binaries will be created in the following path : ${OUT}/data/nativetest64/

To test 64-bit binary push binaries from nativetest64.

adb push ${OUT}/data/nativetest64/writerTest/writerTest /data/local/tmp/

To test 32-bit binary push binaries from nativetest.

adb push ${OUT}/data/nativetest/writerTest/writerTest /data/local/tmp/

The resource file for the tests is taken from [here](https://dl.google.com/android-unittest/media/frameworks/av/media/libstagefright/tests/writer/WriterTestRes-1.2.zip).
Download and extract the folder. Push all the files in this folder to /data/local/tmp/ on the device.
```
adb push WriterTestRes-1.2 /data/local/tmp/
```

usage: writerTest -P \<path_to_res_folder\> -C <remove_output_file>
```
adb shell /data/local/tmp/writerTest -P /data/local/tmp/WriterTestRes-1.2/ -C true
```
Alternatively, the test can also be run using atest command.

```
atest writerTest -- --enable-module-dynamic-download=true
```
