#! /bin/sh

JavaPOS_Sample=../../UareUSampleJavaPOS
Lib_Dir=../../../lib/java

LD_LIBRARY_PATH=$../../../lib/x86 java -cp "$JavaPOS_Sample/src:$JavaPOS_Sample:$JavaPOS_Sample/config:./dpjavapos_app.jar:$Lib_Dir/xmlParserAPIs.jar:$Lib_Dir/xercesImpl-2.6.2.jar:$Lib_Dir/jpos113.jar:$Lib_Dir/dpuareu.jar:$Lib_Dir/dpjavapos.jar" com.digitalpersona.javapos.sampleapp.biometrics.SampleApp $* 
