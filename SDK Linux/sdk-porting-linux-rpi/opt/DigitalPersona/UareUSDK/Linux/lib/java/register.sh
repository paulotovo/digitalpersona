#!/bin/bash
#
# This script can be used to register DigitalPersona U.are.U 4000B Biometrics Device Service 
# in a JavaPOS environment. 

JAVA_POS_CONFIG_PATH=../../Samples/UareUSampleJavaPOS

JAVA=java

CP=$JAVA_POS_CONFIG_PATH:$JAVA_POS_CONFIG_PATH/src:$JAVA_POS_CONFIG_PATH/config:dpjavapos.jar:jpos113.jar:xercesImpl-2.6.2.jar:xmlParserAPIs.jar

$JAVA -cp $CP com.digitalpersona.javapos.services.biometrics.Registration $*

