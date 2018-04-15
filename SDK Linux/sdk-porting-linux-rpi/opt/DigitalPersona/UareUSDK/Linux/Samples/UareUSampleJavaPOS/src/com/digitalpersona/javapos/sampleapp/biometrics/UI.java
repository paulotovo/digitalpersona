package com.digitalpersona.javapos.sampleapp.biometrics;

import jpos.BiometricsControl113;

import java.util.EnumSet;

/**
 * Sample application UI
 */
public interface UI extends Runnable{
    public static enum Operation {
        OPEN ("Open"),
        CHECK_HEALTH ("Check Health"),
        CLOSE ("Close"),
        CLAIM ("Claim"),
        RELEASE ("Release"),
        DEVICE_ENABLE ("Device Enable"),
        DEVICE_DISABLE ("Device Disable"),
        CLEAR_DATA ("Clear Data"),
        BEGIN_ENROLL_CAPTURE ("Begin Enroll Capture"),
        END_CAPTURE ("End Capture"),
        BEGIN_VERIFY_CAPTURE ("Begin Verify Capture"),
        IDENTIFY_MATCH ("Identify Match"),
        VERIFY_MATCH ("Verify Match"),
        IDENTIFY ("Identify"),
        VERIFY ("Verify");

        private String itemText;

        Operation(String itemText) {
            this.itemText = itemText;
        }

        public String getItemText() {
            return itemText;
        }
    }

    public static class OperationEvent {
        private final Object source;
        private final Operation operation;

        public OperationEvent(Object source, Operation operation) {
            this.source = source;
            this.operation = operation;
        }

        public Object getSource() {
            return source;
        }

        public Operation getOperation() {
            return operation;
        }
    }

    public interface OperationListener {
        public void onOperation (OperationEvent event);
    }

    public void displayMessage(String prompt);

    public void logMessage (String message);

    public void setEnabledOperations (EnumSet<Operation> operations);

    public void addOperationListener (OperationListener listener);

    public void removeOperationListener (OperationListener listener);

    public void setBiometricsControl(BiometricsControl113 biometricsControl);
}
