# DigitalPersona Java sample code for U.are.U SDK

LIB_DIR ?= ../../lib/java
DPJAR = dpuareu.jar

all:
	javac -classpath $(LIB_DIR)/$(DPJAR) *.java
	jar -cvf UareUSampleJava.jar *.class

clean:
	rm -f *.jar
	rm -f *.class
	rm -f *~

run:
	LD_LIBRARY_PATH=$(LIB_DIR) java -cp ".:$(LIB_DIR)/$(DPJAR)" UareUSampleJava
