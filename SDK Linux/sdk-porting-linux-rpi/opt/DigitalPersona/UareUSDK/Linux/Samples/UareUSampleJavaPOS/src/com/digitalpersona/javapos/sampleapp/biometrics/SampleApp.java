/**
 * Copyright (c) 2007, DigitalPersona, Inc.
 *
 * File: SampleApp.java
 *
 * Contents: Sample application
 *
 * Remarks:
 */
package com.digitalpersona.javapos.sampleapp.biometrics;

import com.digitalpersona.javapos.services.biometrics.DPFPConstants;
import jpos.*;
import jpos.events.*;

import java.util.*;

public class SampleApp {
	boolean isEnrolling = false;
	int numEnrollCaptures = 0;
	
    /**
     * Instantiates a new sample app.
     * @param ui user interface to use
     */
    public SampleApp(UI ui) {
        this.ui = ui;
        biometricsControl = new Biometrics();
        biometricsControl.addDataListener(new BiometricDataListener());
        biometricsControl.addErrorListener(new BiometricErrorListener());
        biometricsControl.addDirectIOListener(new DeviceDirectIOListener());
        biometricsControl.addStatusUpdateListener(new DeviceStatusUpdateListener());

        ui.setBiometricsControl(biometricsControl);

        ui.addOperationListener(new UI.OperationListener() {
            public void onOperation(UI.OperationEvent event) {
                switch (event.getOperation()) {
                    case BEGIN_ENROLL_CAPTURE:
                        beginCapture(false);
                        break;
                    case BEGIN_VERIFY_CAPTURE:
                        beginCapture(true);
                        break;
                    case CHECK_HEALTH:  
                        checkHealth();
                        break;
                    case CLAIM:
                        claim();
                        break;
                    case CLEAR_DATA:
                        clearData();
                        break;
                    case CLOSE:
                        close();
                        break;
                    case DEVICE_DISABLE:
                        setDeviceEnabled(false);
                        break;
                    case DEVICE_ENABLE:
                        setDeviceEnabled(true);
                        break;
                    case END_CAPTURE:
                        endCapture();
                        break;
                    case IDENTIFY:
                        identify();
                        break;
                    case IDENTIFY_MATCH:
                        identifyMatch();
                        break;
                    case OPEN:
                        open();
                        break;
                    case RELEASE:
                        release();
                        break;
                    case VERIFY:
                        verify();
                        break;
                    case VERIFY_MATCH:
                        verifyMatch();
                        break;
                }
            }
        });

        new Thread(ui).start();
        ui.displayMessage(MSG_CLICK_OPEN);
        updateEnabledOperationList();
    }

    /**
     * The main method.
     *
     * @param args the arguments
     */
    public static void main(String args[]) {
        UI ui = new SwingUI();
        new SampleApp(ui);
    }

    /**
     * Listener for direct IO events from the biometric control.
     */
    private class DeviceDirectIOListener implements DirectIOListener {
        /**
         * Called when direct IO event occurs.
         *
         * @param event event occured
         */
        public void directIOOccurred(DirectIOEvent event) {
            try {
            	String directIOText = lookupDirectIOText(event.getEventNumber());

            	if (directIOText == MSG_SAMPLE_CAPTURED)
            	{
	            	if (isEnrolling)
	            	{
	        			numEnrollCaptures++;
            			log("Application: Enrollment capture count:  " + numEnrollCaptures);            		
	            	}
            	}    
            	
                log("directIOOccurred:" + directIOText);
            } catch (Exception je) {
                logException(je);
            }
        }
    }

    /**
     * Listener for data events from biometric control.
     */
    private class BiometricDataListener implements DataListener {
        /**
         * Called when biometric data captured.
         *
         * @param event event occured.
         */
        public void dataOccurred(DataEvent event) {
            log("BiometricDataListener.dataOccurred():" + lookupDataText(event.getStatus()));
            try {
                switch (event.getStatus()) {
                    case jpos.BiometricsConst.BIO_DATA_ENROLL:
                        if (biometricsControl.getBIR() != null) {
                            referenceBIRPopulation.add(biometricsControl.getBIR().clone());
                        	isEnrolling = false;
                        }
                        ui.displayMessage(MSG_ENROLLMENT_CAPTURED + referenceBIRPopulation.size());
                        break;
                    case jpos.BiometricsConst.BIO_DATA_VERIFY:
                        if (biometricsControl.getBIR() != null) {
                            sampleBIR = biometricsControl.getBIR().clone();
                        }
                        ui.displayMessage(MSG_SAMPLE_CAPTURED);
                        break;
                    default:
                        throw new Exception("EnrollmentDataListener.dataOccurred(): unexpected data event dropped");
                }

                //Update GUI
                inProgress = false;
                isCapturing = false;
                updateEnabledOperationList();
            } catch (Exception ex) {
                logException(ex);
            }
        }
    }

    private class BiometricErrorListener implements ErrorListener {
        /**
         * This method is used to deliver all error events to the application layer
         *
         * @param event error occured
         * @see jpos.events.ErrorListener#errorOccurred(jpos.events.ErrorEvent)
         */
        public void errorOccurred(ErrorEvent event) {
            log(MSG_ERROR_EVNT + lookupErrorText(event.getErrorCode(), event.getErrorCodeExtended()));

            ui.displayMessage(MSG_CAPTURE_FAIL);

    		isEnrolling = false;
            inProgress = false;
            isCapturing = false;
            updateEnabledOperationList();

            try {
                biometricsControl.clearInput();
            } catch (jpos.JposException e) {
                log(e.getMessage());
            }
        }
    }

    /**
     * Listener for direct IO events from the biometric control.
     */
    private class DeviceStatusUpdateListener implements StatusUpdateListener {
        /**
         * Called when direct IO event occurs.
         *
         * @param event event occured
         */
    	public void statusUpdateOccurred(StatusUpdateEvent event)
    	{
            try {
                log("statusUpdateOccurred: rawSensorData received");
            } catch (Exception je) {
                logException(je);
            }    		
    	}
    }

    /**
     * Called when user selects "Open" action in UI.
     * Calls the <code>open()</code> method of Biometric Device Control to initialize the device component.
     */
    public void open() {
        //Set the state variables
        inProgress = true;
        endCaptureCalled = false;
        isCapturing = false;
        updateEnabledOperationList();

        //start a new thread to avoid UI freeze
        new Thread(new Runnable() {
            public void run() {
                ui.displayMessage(MSG_OPENING);
                try {
                    biometricsControl.open(DP_FINGERPRINT_READER_DEVICE_NAME);
                    ui.displayMessage(MSG_OPENED);
                } catch (JposException e1) {
                    ui.displayMessage(MSG_CANT_OPEN);
                    logException(e1);
                }
                inProgress = false;
                updateEnabledOperationList();
            }
        }).start();
    }

    /**
     * Called when user selects "Close" action in UI.
     * This method calls the Biometric Device Control's close for terminating the device component.
     */
    public void close() {
        //Set the state variables
        inProgress = true;
        endCaptureCalled = true;
        updateEnabledOperationList();

        //start a new thread to avoid UI freeze
        new Thread(new Runnable() {
            public void run() {
                ui.displayMessage(MSG_CLOSING);
                try {
                    biometricsControl.close();
                    clearSampleTemplate();
                    referenceBIRPopulation.clear();
                    ui.displayMessage(MSG_CLICK_OPEN);
                    updateEnabledOperationList();
                } catch (JposException e1) {
                    ui.displayMessage(MSG_CANT_CLOSE);
                    logException(e1);
                }
                inProgress = false;
                updateEnabledOperationList();
            }
        }).start();
    }

    /**
     * Called when user selects "Claim" action in UI.
     * Calls the <code>claim()</code> method of Biometric Device Control to claim the device.
     */
    public void claim() {
        //Set the state variables
        inProgress = true;
        updateEnabledOperationList();

        //start a new thread to avoid UI freeze
        new Thread(new Runnable() {
            public void run() {
                ui.displayMessage(MSG_CLAIMING + timeout + MSG_CLAIM_TIOMEOUT);
                try {
                    biometricsControl.claim(timeout);
                    ui.displayMessage(MSG_CLAIMED);
                } catch (JposException e1) {
                    ui.displayMessage(MSG_CANT_CLAIM);
                    logException(e1);
                }
                inProgress = false;
                updateEnabledOperationList();
            }
        }).start();
    }

    /**
     * Called when user selects "Release" action in UI.
     * Calls the <code>release()</code> method of Biometric Device Control to release the device.
     */
    public void release() {
        inProgress = true;
        endCaptureCalled = true;
        updateEnabledOperationList();

        new Thread(new Runnable() {
            public void run() {
                try {
                    ui.displayMessage(MSG_RELEASING);
                    biometricsControl.setDataEventEnabled(false);
                    biometricsControl.release();
                    biometricsControl.clearInput();
                    ui.displayMessage(MSG_RELEASED);
                } catch (JposException e1) {
                    ui.displayMessage(MSG_CANT_RELEASE);
                    logException(e1);
                }
                inProgress = false;
                updateEnabledOperationList();
            }
        }).start();
    }

    /**
     * Called when user selects "Verify" action in UI.
     * Calls the <code>verify()</code> method of Biometric Device Control for synchronous verification.
     */
    public void verify() {
        final byte[] enrollData = getLastEnrollment();
        if (enrollData == null) {
            ui.displayMessage(MSG_NOENROLL_EXIST);
            return;
        }

        clearSampleTemplate();

        inProgress = true;
        endCaptureCalled = false;

        //Update GUI
        ui.displayMessage(MSG_TOUCH_FOR_VERIFY);
        updateEnabledOperationList();

        //start a new thread to avoid UI freeze
        new Thread(new Runnable() {
            public void run() {
                try {
                    //Clear buffered events
                    biometricsControl.clearInput();

                    //Allow events to be delivered to application
                    biometricsControl.setDataEventEnabled(true);

                    //Start synchronous verification
                    boolean[] result = new boolean[1];
                    biometricsControl.verify(maxFARRequested,
                            maxFRRRequested, FARPrecedence,
                            enrollData, adaptedBIR, result,
                            FARAchieved, FRRAchieved, payload, timeout);

                    if (result[0]) {
                        ui.displayMessage(MSG_VERIFY_SUCCESS);
                    } else if (!endCaptureCalled) {
                        ui.displayMessage(MSG_VERIFY_FAILED);
                    }
                } catch (JposException e) {
                    if (e.getErrorCode() == JposConst.JPOS_E_TIMEOUT) {
                        ui.displayMessage(MSG_OPERATION_TIMEOUT);
                        log(MSG_OPERATION_TIMEOUT);
                    } else {
                        ui.displayMessage(MSG_VERIFY_FAILED);
                        logException(e);
                    }
                }

                try {
                    biometricsControl.setDataEventEnabled(false);
                } catch (JposException e) {
                    logException(e);
                }

                inProgress = false;
                endCaptureCalled = false;
                updateEnabledOperationList();
            }
        }).start();
    }

    /**
     * Called when user selects "Identify" action in UI.
     * Calls the <code>identify()</code> method of Biometric Device Control for synchronous identification.
     */
    public void identify() {
        final byte[][] enrollArray = getEnrollmentsArray();
        if (enrollArray == null) {
            ui.displayMessage(MSG_NOENROLL_EXIST);
            return;
        }

        clearSampleTemplate();

        inProgress = true;
        endCaptureCalled = false;

        //Update GUI
        ui.displayMessage(MSG_TOUCH_FOR_IDENTIFY);
        updateEnabledOperationList();

        //start a new thread to avoid UI freeze
        new Thread(new Runnable() {
            public void run() {
                try {
                    //Clear buffered events
                    biometricsControl.clearInput();

                    //Allow events to be delivered to application
                    biometricsControl.setDataEventEnabled(true);

                    candidateRanking = new int[enrollArray.length][];

                    //Start synchronous identification
                    biometricsControl.identify(maxFARRequested,
                            maxFRRRequested, FARPrecedence, enrollArray,
                            candidateRanking, timeout);

                    String rankingString = getRankingString();
                    if (candidateRanking[0] != null && candidateRanking[0].length > 0) {
                        //Success
                        ui.displayMessage(MSG_IDENTIFY_SUCCESS);
                    } else {
                        //No candidates returned
                        ui.displayMessage(MSG_IDENTIFY_FAILED);
                    }
                    log(rankingString);
                } catch (JposException e) {
                    if (e.getErrorCode() == JposConst.JPOS_E_TIMEOUT) {
                        ui.displayMessage(MSG_OPERATION_TIMEOUT);
                        log(MSG_OPERATION_TIMEOUT);
                    } else {
                        ui.displayMessage(MSG_IDENTIFY_FAILED);
                        logException(e);
                    }
                }

                try {
                    biometricsControl.setDataEventEnabled(false);
                } catch (JposException e) {
                    logException(e);
                }
                candidateRanking[0] = null;

                //Update UI
                inProgress = false;
                endCaptureCalled = false;
                updateEnabledOperationList();
            }
        }).start();
    }

    /**
     * Called when user selects "Verify Match" action in UI.
     * Calls the <code>verifyMatch()</code> method of Biometric Device Control for offline verification.
     */
    public void verifyMatch() {
        final byte[] enrollData = getLastEnrollment();
        if (enrollData == null) {
            ui.displayMessage(MSG_NOENROLL_EXIST);
            return;
        }

        if (sampleBIR.length == 0) {
            ui.displayMessage(MSG_IDENTIFY_FST);
            return;
        }

        inProgress = true;
        endCaptureCalled = false;

        //Update GUI
        ui.displayMessage(MSG_VERIFY_MATCHING);
        updateEnabledOperationList();

        //start a new thread to avoid UI freeze
        new Thread(new Runnable() {
            public void run() {
                try {
                    boolean[] result = new boolean[1];
                    result[0] = false;
                    biometricsControl.verifyMatch(maxFARRequested,
                            maxFRRRequested, FARPrecedence, sampleBIR,
                            enrollData, adaptedBIR, result, FARAchieved,
                            FRRAchieved, payload);
                    if (result[0]) {
                        ui.displayMessage(MSG_VERIFY_SUCCESS);
                    } else {
                        ui.displayMessage(MSG_VERIFY_FAILED);
                    }
                    clearSampleTemplate();
                } catch (JposException e1) {
                    ui.displayMessage(MSG_VERIFY_FAILED);
                    logException(e1);
                }

                //Update UI
                inProgress = false;
                endCaptureCalled = false;
                updateEnabledOperationList();
            }
        }).start();
    }

    /**
     * Called when user selects "Identify Match" action in UI.
     * Calls the <code>identifyMatch()</code> method of Biometric Device Control for offline identification.
     */
    public void identifyMatch() {
        final byte[][] enrollArray = getEnrollmentsArray();
        if (enrollArray == null) {
            ui.displayMessage(MSG_NOENROLL_EXIST);
            return;
        }

        inProgress = true;
        endCaptureCalled = false;

        //Update GUI
        ui.displayMessage(MSG_IDENTIFY_MATCHING);
        updateEnabledOperationList();

        //start a new thread to avoid UI freeze
        new Thread(new Runnable() {
            public void run() {
                try {
                    biometricsControl.setDataEventEnabled(true);
                    candidateRanking = new int[enrollArray.length][];

                    biometricsControl.identifyMatch(maxFARRequested,
                            maxFRRRequested, FARPrecedence, sampleBIR,
                            enrollArray, candidateRanking);

                    String rankingString = getRankingString();
                    if (candidateRanking[0] != null && candidateRanking[0].length > 0) {
                        //Success
                        ui.displayMessage(MSG_IDENTIFY_SUCCESS);
                    } else {
                        //No candidates returned
                        ui.displayMessage(MSG_IDENTIFY_FAILED);
                    }
                    log(rankingString);

                    clearSampleTemplate();
                } catch (JposException e1) {
                    ui.displayMessage(MSG_IDENTIFY_FAILED);
                    logException(e1);
                }

                try {
                    biometricsControl.setDataEventEnabled(false);
                } catch (JposException e) {
                    logException(e);
                }

                //Update UI
                inProgress = false;
                endCaptureCalled = false;
                updateEnabledOperationList();
            }
        }).start();
    }

    /**
     * Called when the user presses the EndCapture button.
     * This method calls the Biometric Device Control's endCapture to terminates the
     * process of acquiring registration or sample template.
     */
    public void endCapture() {
        //Update the GUI state
        inProgress = true;
        endCaptureCalled = true;
        updateEnabledOperationList();

        //start a new thread to avoid UI freeze
        new Thread(new Runnable() {
            public void run() {
                try {
                    //Tell the device to stop capturing
                    biometricsControl.endCapture();

                    //Disable data events
                    try {
                        biometricsControl.setDataEventEnabled(false);
                    } catch (JposException ex) {
                        logException(ex);
                    }

                    //Restore application state
                    inProgress = false;
                    isCapturing = false;
                    isEnrolling = false;

                    //Update the GUI
                    updateEnabledOperationList();
                    ui.displayMessage(MSG_CAPTURE_TERM);
                } catch (JposException ex) {
                    updateEnabledOperationList();
                    logException(ex);
                }
            }
        }).start();
    }

    /**
     * Called when user selects "Begin Verify Capture" or "Begin Enroll Capture" action in UI.
     * Calls the <code>beginVerifyCapture()</code> or <code>beginEnrollCapture()</code> methods
     * of Biometric Device Control to start asynchronous capture.
     *
     * @param verify verification sample capture will be started if <code>true</code>, enroll capture otherwise.
     * @see jpos.events.DataListener
     * @see com.digitalpersona.javapos.sampleapp.biometrics.SampleApp.BiometricDataListener#dataOccurred(jpos.events.DataEvent)
     */
    public void beginCapture(final boolean verify) {
        if (verify && sampleBIR.length > 0) {
            clearSampleTemplate();
        }

        //Update the GUI state
        inProgress = true;
        isCapturing = true;
        updateEnabledOperationList();

        //start a new thread to avoid UI freeze
        new Thread(new Runnable() {
            public void run() {
                try {
                    //Clear buffered events
                    biometricsControl.clearInput();

                    //Allow events to be delivered to application
                    biometricsControl.setDataEventEnabled(true);

                    //Start capturing
                    if (verify)
                        biometricsControl.beginVerifyCapture();
                    else
                    {
                    	isEnrolling = true;
                    	numEnrollCaptures = 0;
                    	biometricsControl.beginEnrollCapture(refBir, payLoad);
                    }                        

                    //Display user prompt
                    ui.displayMessage(verify ? MSG_TOUCH_FORSAMPLE : MSG_TOUCH_SENS);
                    updateEnabledOperationList();
                } catch (JposException ex) {
                    ui.displayMessage(MSG_CAPTURE_FAIL);

                    //update GUI state
                    inProgress = false;
                    isCapturing = false;
                    isEnrolling = false;
                    updateEnabledOperationList();

                    logException(ex);

                    try {
                        biometricsControl.setDataEventEnabled(false);
                    } catch (JposException ex1) {
                        logException(ex1);
                    }
                }
            }
        }).start();
    }

    /**
     * Called when user selects "Clear Data" action in UI.
     */
    public void clearData() {
        clearSampleTemplate();
        referenceBIRPopulation.clear();
        ui.displayMessage(MSG_CLEAR);
        updateEnabledOperationList();
    }

    /**
     * Called when user selects "Device Enable" or "Device Disable" action in UI.
     * Calls the <code>setDeviceEnabled()</code> method of Biometric Device
     * Control to enable or disable the device.
     *
     * @param state the desired state
     */
    public void setDeviceEnabled(final boolean state) {
        inProgress = true;
        updateEnabledOperationList();

        new Thread(new Runnable() {
            public void run() {
                ui.displayMessage(state ? MSG_DEVICE_ENABLING : MSG_DEVICE_DISABLING);
                try {
                    biometricsControl.setDeviceEnabled(state);
                    ui.displayMessage(state ? MSG_DEVICE_ENABLED : MSG_DEVICE_DISABLED);
                } catch (JposException ex) {
                    ui.displayMessage(state ? MSG_CANT_ENABLE : MSG_CANT_DISABLE);
                    logException(ex);
                }
                inProgress = false;
                updateEnabledOperationList();
            }
        }).start();
    }

    /**
     * Called when user selects "Check Health" action in UI.
     * Calls the <code>checkHealth()</code> method of Biometric Device
     * Control with all possible arguments to check the device helth.
     */
    public void checkHealth() {
        Map<Integer, String> levels = new HashMap<Integer, String>();
        levels.put (JposConst.JPOS_CH_INTERNAL, "Internal health check:");
        levels.put (JposConst.JPOS_CH_EXTERNAL, "External health check:");
        levels.put (JposConst.JPOS_CH_INTERACTIVE, "Interactive health check:");

        for (int level : levels.keySet()) {
            log (levels.get(level));
            try {
                biometricsControl.checkHealth(level);
            } catch (JposException ex) {
                if (ex.getErrorCode() == JposConst.JPOS_E_ILLEGAL) {
                    log("Not supported.");
                } else {
                    logException(ex);
                }
            }
            catch (Exception ex) {
                logException(ex);
            }

            try {
                log("getCheckHealthText() = \"" + biometricsControl.getCheckHealthText() + "\"\n");
            } catch (JposException e) {
                logException(e);
            }
        }
    }

    /**
     * This method sets the state of buttons enable/disable.
     */
    private void updateEnabledOperationList() {
        boolean opened = biometricsControl.getState() != JposConst.JPOS_S_CLOSED;
        boolean claimed = false;
        boolean enabled = false;
        try {
            claimed = biometricsControl.getClaimed();
            enabled = biometricsControl.getDeviceEnabled();
        } catch (JposException e) {
        }

        EnumSet<UI.Operation> operations = EnumSet.noneOf(UI.Operation.class);
        if (isCapturing) { //beginEnrollCapture or beginVerifyCapture called
            operations.addAll(EnumSet.of(UI.Operation.CHECK_HEALTH, UI.Operation.CLOSE, UI.Operation.RELEASE, UI.Operation.DEVICE_DISABLE, UI.Operation.END_CAPTURE));
        } else if (inProgress) {
        } else if (!opened) {//Closed
            operations.add(UI.Operation.OPEN);
        } else if (!claimed) {//Opened
            operations.addAll(EnumSet.of(UI.Operation.CHECK_HEALTH, UI.Operation.CLOSE, UI.Operation.CLAIM));
        } else if (!enabled) { //Claimed
            operations.addAll(EnumSet.of(UI.Operation.CHECK_HEALTH, UI.Operation.CLOSE, UI.Operation.RELEASE, UI.Operation.DEVICE_ENABLE));
        } else {//Enabled
            operations.addAll(EnumSet.of(UI.Operation.CHECK_HEALTH, UI.Operation.CLOSE, UI.Operation.RELEASE, UI.Operation.DEVICE_DISABLE, UI.Operation.BEGIN_ENROLL_CAPTURE, UI.Operation.BEGIN_VERIFY_CAPTURE));
            if (!referenceBIRPopulation.isEmpty()) {
                operations.addAll(EnumSet.of(UI.Operation.VERIFY, UI.Operation.IDENTIFY, UI.Operation.CLEAR_DATA));
                if (sampleBIR != null && sampleBIR.length > 0) {
                    operations.addAll(EnumSet.of(UI.Operation.VERIFY_MATCH, UI.Operation.IDENTIFY_MATCH));
                }
            }
        }

        ui.setEnabledOperations(operations);
    }

    /**
     * This method is used to get the enrollment template.
     *
     * @return array of previously enrolled BIRs
     */
    private byte[][] getEnrollmentsArray() {
        if (referenceBIRPopulation.size() > 0) {
            byte[][] arBytes = new byte[referenceBIRPopulation.size()][];
            for (int i = 0; i < referenceBIRPopulation.size(); i++) {
                arBytes[i] = referenceBIRPopulation.get(i);
            }
            return arBytes;
        }
        return null;
    }

    /**
     * This method is used to get the last enrollment template.
     *
     * @return Most recently enrolled BIR
     */
    private byte[] getLastEnrollment() {
        byte[][] arTemplates = getEnrollmentsArray();
        if (arTemplates != null) {
            return arTemplates[arTemplates.length - 1];
        }
        return null;
    }

    /**
     * This method is used to clear the sampleBIR template.
     */
    private void clearSampleTemplate() {
        if (sampleBIR.length > 0) {
            sampleBIR = new byte[0];
        }
    }

    /**
     * Returns printable list of candidate ranks
     *
     * @return printable list of candidate ranks
     */
    private String getRankingString() {
        StringBuilder rankingStr = new StringBuilder();
        if (candidateRanking != null && candidateRanking[0] != null && candidateRanking[0].length > 0) {
            rankingStr.append(MSG_RANKING);
            for (int i = 0; i < candidateRanking[0].length; i++) {
                if (i > 0)
                    rankingStr.append(", ");
                rankingStr.append(candidateRanking[0][i]);
            }
        } else {
            rankingStr.append(MSG_NO_RANKING_FOUND);
        }

        return rankingStr.toString();
    }

    /**
     * Logs the message.
     *
     * @param message string to log
     */
    protected void log(String message) {
        ui.logMessage(LOG_PREFIX + message);
    }

    /**
     * Logs the exception.
     *
     * @param e exception to log.
     */
    private void logException(Exception e) {
        if (e instanceof JposException) {
            JposException je = (JposException) e;
            String errorText = lookupErrorText(je.getErrorCode(), je.getErrorCodeExtended());
            if (errorText.length() == 0) {
                log(je.getMessage());
            } else {
                log(errorText);
            }
        } else {
            log(MSG_EXCEP_OCCURRED + e.getMessage());
        }
    }


    private String lookupDataText(int code) {
        switch (code) {
            case BiometricsConst.BIO_DATA_ENROLL:
                return MSG_ENROLL_DATA;
            case BiometricsConst.BIO_DATA_VERIFY:
                return MSG_VERIFY_DATA;
            default:
                return "" + code;
        }
    }

    private String lookupDirectIOText(int code) {
        switch (code) {
            case DPFPConstants.DP_EVENT_DISCONNECT:
                return MSG_DEVICE_DISCONN;
            case DPFPConstants.DP_EVENT_RECONNECT:
                return MSG_DEVICE_RECONN;
            case DPFPConstants.DP_EVENT_FINGER_TOUCHED:
                return MSG_FINGER_TOUCHED;
            case DPFPConstants.DP_EVENT_FINGER_GONE:
                return MSG_FINGER_GONE;
            case 7:
                return MSG_SAMPLE_CAPTURED;
            case 10:
                return MSG_CAPTURE_TERM;
            case 4:
                return MSG_BAD_FINGERPRINT;
            default:
                return "" + code;
        }
    }


    private String lookupErrorText(int code, int extendedCode) {
        switch (code) {
            case JposConst.JPOS_E_CLAIMED:
                return MSG_CANT_CLAIM;
            case JposConst.JPOS_E_CLOSED:
                return MSG_DEVICE_CLOSED;
            case JposConst.JPOS_E_DISABLED:
                return MSG_DEVICE_DISABLED;
            case JposConst.JPOS_E_EXTENDED:
                return MSG_EXTENDED_ERROR_CODE + extendedCode;
            case JposConst.JPOS_E_FAILURE:
                return MSG_OPERATION_FAILED;
            case JposConst.JPOS_E_ILLEGAL:
                return MSG_OPERATION_ILLEGAL;
            case JposConst.JPOS_E_NOHARDWARE:
                return MSG_NO_HARDWARE;
            case JposConst.JPOS_E_NOSERVICE:
                return MSG_NO_SERVICE;
            case JposConst.JPOS_E_NOTCLAIMED:
                return MSG_NOT_CLAIMED;
            case JposConst.JPOS_E_TIMEOUT:
                return MSG_OPERATION_TIMEOUT;
            case DPFPConstants.DP_EVENT_ENROLL_FAIL:
            	if (extendedCode == DPFPConstants.DPFJ_E_ENROLLMENT_INVALID_SET)
            	{
            		isEnrolling = false;
            		return MSG_UNSUCCESSFUL_ENROLLMENT;
            	}
            default:
                return "";
        }
    }

    /**
     * Boolean variables used to check the operation in progress
     */
    private boolean inProgress = false;
    private boolean endCaptureCalled = false;
    private boolean isCapturing = false;

    /**
     * The logicalName of the device.
     */
    private String DP_FINGERPRINT_READER_DEVICE_NAME = "DPFingerprintReader";

    /**
     * Biometrics Control.
     */
    private BiometricsControl113 biometricsControl;

    /**
     * The UI which runs the application
     */
    private final UI ui;

    /**
     * Holds refBir information.
     */
    private byte[] refBir = {};

    /**
     * Holds payLoad information.
     */
    private byte[] payLoad = {};

    /**
     * Holds maxFARRequested information.
     */
    private int maxFARRequested = 2147483;

    /**
     * Holds maxFRRRequested information.
     */
    private int maxFRRRequested = 1;

    /**
     * Holds FARPrecedence information.
     */
    private boolean FARPrecedence = true;

    /**
     * Holds sampleBIR information.
     */
    private byte[] sampleBIR = {};

    /**
     * Holds referenceBIRPopulation information.
     */
    private List<byte[]> referenceBIRPopulation = new ArrayList<byte[]>();

    /**
     * Holds candidateRanking information.
     */
    private int[][] candidateRanking = {};

    /**
     * Holds adaptedBIR information.
     */
    private byte[][] adaptedBIR = {};

    /**
     * Holds FARAchieved information.
     */
    private int[] FARAchieved = {};

    /**
     * Holds FRRAchieved information.
     */
    private int[] FRRAchieved = {};

    /**
     * Holds payload information.
     */
    private byte[][] payload = {};

    /**
     * Holds timeout information.
     */
    private static final int timeout = 10000;

    /**
     * Application level log prefix.
     */
    private static final String LOG_PREFIX = "Application: ";

    /**
     * Message constants.
     */
    private static final String MSG_CLICK_OPEN = "Click Open to open the device...";

    private static final String MSG_OPENING = "Opening the device...";
    private static final String MSG_OPENED = "Device opened.";
    private static final String MSG_CANT_OPEN = "Can't open the device.";

    private static final String MSG_CLOSING = "Closing the device...";
    private static final String MSG_CANT_CLOSE = "Can't close the device.";

    private static final String MSG_CLAIMING = "Claiming the device with timeout ";
    private static final String MSG_CLAIM_TIOMEOUT = " milliseconds...";
    private static final String MSG_CLAIMED = "Exclusive access established.";
    private static final String MSG_CANT_CLAIM = "Can't claim the device.";

    private static final String MSG_RELEASING = "Releasing the device...";
    private static final String MSG_RELEASED = "Released exclusive access to the device.";
    private static final String MSG_CANT_RELEASE = "Can't release the device.";

    private static final String MSG_DEVICE_ENABLING = "Enabling the device.";
    private static final String MSG_DEVICE_ENABLED = "Device enabled.";
    private static final String MSG_CANT_ENABLE = "Can't enable the device.";

    private static final String MSG_DEVICE_DISABLING = "Disabling the device.";
    private static final String MSG_CANT_DISABLE = "Can't disable the device.";

    private static final String MSG_TOUCH_SENS = "Touch the sensor a minimum of four times...";
    private static final String MSG_ENROLLMENT_CAPTURED = "Total enrollment completed: ";

    private static final String MSG_TOUCH_FORSAMPLE = "Touch the sensor to capture sample data...";
    private static final String MSG_SAMPLE_CAPTURED = "Sample data captured.";

    private static final String MSG_CAPTURE_TERM = "Biometrics capture is terminated.";

    private static final String MSG_NOENROLL_EXIST = "No Enrollment data exist! Please do the enrollment first.";

    private static final String MSG_VERIFY_SUCCESS = "Verification success!";
    private static final String MSG_VERIFY_FAILED = "Verification failed!";

    private static final String MSG_IDENTIFY_SUCCESS = "Identification Success!";
    private static final String MSG_IDENTIFY_FAILED = "Identification Failed!";
    private static final String MSG_RANKING = "Candidate Ranking: ";
    private static final String MSG_NO_RANKING_FOUND = "Matching Finger not found! No Candidate Ranking Found.";

    private static final String MSG_TOUCH_FOR_IDENTIFY = "Please touch the sensor for Identification...";
    private static final String MSG_TOUCH_FOR_VERIFY = "Please touch the sensor for verification...";
    private static final String MSG_CAPTURE_FAIL = "Capture failed, please try again.";

    private static final String MSG_VERIFY_MATCHING = "Verify matching...";
    private static final String MSG_IDENTIFY_MATCHING = "Identify matching...";

    private static final String MSG_EXCEP_OCCURRED = "Exception: ";

    private static final String MSG_FINGER_TOUCHED = "Finger touched.";
    private static final String MSG_FINGER_GONE = "Finger gone.";
    private static final String MSG_DEVICE_DISCONN = "Device disconnect.";
    private static final String MSG_DEVICE_RECONN = "Device reconnect.";

    private static final String MSG_BAD_FINGERPRINT = "Bad fingerprint sample quality, please try again...";

    private static final String MSG_CLEAR = "Data cleared.";
    private static final String MSG_ERROR_EVNT = "Error Event: ";
    private static final String MSG_IDENTIFY_FST = "Please do Begin Verify Capture first!";
    private static final String MSG_DEVICE_CLOSED = "Device closed.";
    private static final String MSG_DEVICE_DISABLED = "Device disabled.";
    private static final String MSG_EXTENDED_ERROR_CODE = "Extended error code = ";
    private static final String MSG_OPERATION_FAILED = "Operation failed.";
    private static final String MSG_OPERATION_ILLEGAL = "Operation is not allowed at this time.";
    private static final String MSG_NO_HARDWARE = "No hardware.";
    private static final String MSG_NO_SERVICE = "No service.";
    private static final String MSG_NOT_CLAIMED = "Device is not claimed.";
    private static final String MSG_OPERATION_TIMEOUT = "Operation timeout.";
    private static final String MSG_ENROLL_DATA = "Enrollment data.";
    private static final String MSG_VERIFY_DATA = "Verification data.";
    
    private static final String MSG_UNSUCCESSFUL_ENROLLMENT = "Enrollment was unsuccessful.";
    
}

