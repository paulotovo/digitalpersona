# DigitalPersona Java sample code for U.are.U SDK

LIB_DIR ?= ../../lib/java
CLASS_DIR ?= ./src/com/digitalpersona/javapos/sampleapp/biometrics
JAR_NAME ?= dpjavapos_app.jar

all:
	javac -classpath $(LIB_DIR)/dpjavapos.jar:$(LIB_DIR)/jpos113.jar:$(LIB_DIR)/xercesImpl-2.6.2.jar:$(LIB_DIR)/xmlParserAPIs.jar $(CLASS_DIR)/*.java 

	jar -cvf $(JAR_NAME) $(CLASS_DIR)/*.class

clean:
	rm -f *.jar
	rm -f $(CLASS_DIR)/*.class
	rm -f *~

run:
	LD_LIBRARY_PATH=../../../lib/x86 java -cp "./src:.:./config:dpjavapos_app.jar:$(LIB_DIR)/xmlParserAPIs.jar:$(LIB_DIR)/xercesImpl-2.6.2.jar:$(LIB_DIR)/jpos113.jar:$(LIB_DIR)/dpuareu.jar:$(LIB_DIR)/dpjavapos.jar" com.digitalpersona.javapos.sampleapp.biometrics.SampleApp

