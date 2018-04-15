/**
 * Copyright (c) 2007-2008, DigitalPersona, Inc.
 *
 * File: JButtonsPanel.java
 *
 * Contents: Button panel for SampleApp
 *
 * Remarks:
 */
package com.digitalpersona.javapos.sampleapp.biometrics;

import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionListener;

public class JButtonsPanel extends JPanel {
	private static final long serialVersionUID = 1L;

	/** The jOpen used to open the device. */
    private JButton jOpen = new JButton();

    /** The jClaim used to claim the device. */
    private JButton jClaim = new JButton();

    /** The jDeviceEnable used for enabling device. */
    private JButton jDeviceEnable = new JButton();

    /** The jDeviceDisable used for disabling the device. */
    private JButton jDeviceDisable = new JButton();

    /** The jClearDate used for disabling clearing the Enrolment Templates as well as Sample Template. */
    private JButton jClearData = new JButton();

    /** The jBeginCapture used for starting the device to capture the biometric data. */
    private JButton jBeginEnrollCapture = new JButton();

    /** The jBeginVarifyCapture used for starting the device to capture the biometric data. */
    private JButton jBeginVerifyCapture = new JButton();

    /** The jEndCapture used to end the capturing of the biometric data. */
    private JButton jEndCapture = new JButton();

    /** The jIdentify used for identification of the biometric data. */
    private JButton jIdentify = new JButton();

    /** The jIdentifyMatch used for identification of the biometric data. */
    private JButton jIdentifyMatch = new JButton();

    /** The jVerifyMatch used for the verification of the biometric data. */
    private JButton jVerifyMatch = new JButton();

    /** The jVerify used to verify the biometric data. */
    private JButton jVerify = new JButton();

    /** The jRelease is used to release the biometric device. */
    private JButton jRelease = new JButton();

    /** The jClose is used to close the biometric device. */
    private JButton jClose = new JButton();

    /** The jClose is used to close the biometric device. */
    private JButton jCheckHealth = new JButton();

    /* Button names. */
    static final String BTN_OPEN = "Open";
    static final String BTN_CLOSE = "Close";
    static final String BTN_CLAIM = "Claim";
    static final String BTN_RELEASE = "Release";
    static final String BTN_DEVICE_ENABLE = "Device Enable";
    static final String BTN_DEVICE_DISABLE = "Device Disable";
    static final String BTN_END_CAPTURE = "End Capture";
    static final String BTN_BEGIN_ENROLL_CAPTURE = "Begin Enroll Capture";
    static final String BTN_BEGIN_VERIFY_CAPTURE = "Begin Verify Capture";
    static final String BTN_IDENTIFY_MATCH = "Identify Match";
    static final String BTN_VERIFY_MATCH = "Verify Match";
    static final String BTN_IDENTIFY = "Identify";
    static final String BTN_VERIFY = "Verify";
    static final String BTN_CLEAR_DATA = "Clear Data";
    static final String BTN_CHECK_HEALTH = "Check Health";

    /* Button hints. */
    static final String MSG_OPEN = "Open the device";
    static final String MSG_CLOSE = "Close the device";
    static final String MSG_CLAIM = "Claim the device for exclusive access";
    static final String MSG_RELEASE = "Release exclusively claimed device";
    static final String MSG_DEVICE_ENABLE = "Enable the device";
    static final String MSG_DEVICE_DISABLE = "Disable the device";
    static final String MSG_END_CAPTURE = "End data capturing";
    static final String MSG_BEGIN_ENROLL_CAPTURE = "Activates the device to begin data capturing";
    static final String MSG_BEGIN_VERIFY_CAPTURE = "Starts capturing biometrics data for the purposes of verification";
    static final String MSG_IDENTIFY_MATCH = "Used for data comparison purposes";
    static final String MSG_VERIFY_MATCH = "Used for data matching purposes";
    static final String MSG_IDENTIFY = "Used for synchronous biometric identifications";
    static final String MSG_VERIFY = "Used for synchronous biometric verifications";
    static final String MSG_CLEAR_DATA = "Clears the Template";
    static final String MSG_CHECK_HEALTH = "Checks the Device Health";

    /* Button rectangles */
    private static final int BTN_HEIGHT = 25;
    private static final int BTN_WIDTH = 150;
    private static final int BTN_LEFT = 4;
    private static final int BTN_VSPACE = 2;

    /**
     * Constructs the panel and initializes the buttons.
     */
    public JButtonsPanel() {
        setBorder(BorderFactory.createEtchedBorder(Color.GREEN, new Color(148, 145, 140)));

        jOpen.setText(BTN_OPEN);
        jOpen.setToolTipText(MSG_OPEN);
        jOpen.setBounds(new Rectangle(BTN_LEFT, BTN_VSPACE, BTN_WIDTH, BTN_HEIGHT));
        add(jOpen);

        jCheckHealth.setText(BTN_CHECK_HEALTH);
        jCheckHealth.setToolTipText(MSG_CHECK_HEALTH);
        jCheckHealth.setBounds(new Rectangle(BTN_LEFT, (int) (jOpen.getBounds().getMaxY() + BTN_VSPACE), BTN_WIDTH, BTN_HEIGHT));
        add(jCheckHealth);

        jClose.setText(BTN_CLOSE);
        jClose.setToolTipText(MSG_CLOSE);
        jClose.setBounds(new Rectangle(BTN_LEFT, (int) (jCheckHealth.getBounds().getMaxY() + BTN_VSPACE), BTN_WIDTH, BTN_HEIGHT));
        add(jClose);

        jClaim.setText(BTN_CLAIM);
        jClaim.setToolTipText(MSG_CLAIM);
        jClaim.setBounds(new Rectangle(BTN_LEFT, (int) (jClose.getBounds().getMaxY() + BTN_VSPACE), BTN_WIDTH, BTN_HEIGHT));
        add(jClaim);

        jRelease.setText(BTN_RELEASE);
        jRelease.setToolTipText(MSG_RELEASE);
        jRelease.setBounds(new Rectangle(BTN_LEFT, (int) (jClaim.getBounds().getMaxY() + BTN_VSPACE), BTN_WIDTH, BTN_HEIGHT));
        add(jRelease);

        jDeviceEnable.setText(BTN_DEVICE_ENABLE);
        jDeviceEnable.setToolTipText(MSG_DEVICE_ENABLE);
        jDeviceEnable.setBounds(new Rectangle(BTN_LEFT, (int) (jRelease.getBounds().getMaxY() + BTN_VSPACE), BTN_WIDTH, BTN_HEIGHT));
        add(jDeviceEnable);

        jDeviceDisable.setText(BTN_DEVICE_DISABLE);
        jDeviceDisable.setToolTipText(MSG_DEVICE_DISABLE);
        jDeviceDisable.setBounds(new Rectangle(BTN_LEFT, (int) (jDeviceEnable.getBounds().getMaxY() + BTN_VSPACE), BTN_WIDTH, BTN_HEIGHT));
        add(jDeviceDisable);

        jClearData.setText(BTN_CLEAR_DATA);
        jClearData.setToolTipText(MSG_CLEAR_DATA);
        jClearData.setBounds(new Rectangle(BTN_LEFT, (int) (jDeviceDisable.getBounds().getMaxY() + BTN_VSPACE), BTN_WIDTH, BTN_HEIGHT));
        add(jClearData);

        jBeginEnrollCapture.setText(BTN_BEGIN_ENROLL_CAPTURE);
        jBeginEnrollCapture.setToolTipText(MSG_BEGIN_ENROLL_CAPTURE);
        jBeginEnrollCapture.setBounds(new Rectangle(BTN_LEFT, (int) (jClearData.getBounds().getMaxY() + BTN_VSPACE), BTN_WIDTH, BTN_HEIGHT));
        add(jBeginEnrollCapture);

        jEndCapture.setText(BTN_END_CAPTURE);
        jEndCapture.setToolTipText(MSG_END_CAPTURE);
        jEndCapture.setBounds(new Rectangle(BTN_LEFT, (int) (jBeginEnrollCapture.getBounds().getMaxY() + BTN_VSPACE), BTN_WIDTH, BTN_HEIGHT));
        add(jEndCapture);

        jBeginVerifyCapture.setText(BTN_BEGIN_VERIFY_CAPTURE);
        jBeginVerifyCapture.setToolTipText(MSG_BEGIN_VERIFY_CAPTURE);
        jBeginVerifyCapture.setBounds(new Rectangle(BTN_LEFT, (int) (jEndCapture.getBounds().getMaxY() + BTN_VSPACE), BTN_WIDTH, BTN_HEIGHT));
        add(jBeginVerifyCapture);

        jIdentifyMatch.setText(BTN_IDENTIFY_MATCH);
        jIdentifyMatch.setToolTipText(MSG_IDENTIFY_MATCH);
        jIdentifyMatch.setBounds(new Rectangle(BTN_LEFT, (int) (jBeginVerifyCapture.getBounds().getMaxY() + BTN_VSPACE), BTN_WIDTH, BTN_HEIGHT));
        add(jIdentifyMatch);

        jVerifyMatch.setText(BTN_VERIFY_MATCH);
        jVerifyMatch.setToolTipText(MSG_VERIFY_MATCH);
        jVerifyMatch.setBounds(new Rectangle(BTN_LEFT, (int) (jIdentifyMatch.getBounds().getMaxY() + BTN_VSPACE), BTN_WIDTH, BTN_HEIGHT));
        add(jVerifyMatch);

        jIdentify.setText(BTN_IDENTIFY);
        jIdentify.setToolTipText(MSG_IDENTIFY);
        jIdentify.setBounds(new Rectangle(BTN_LEFT, (int) (jVerifyMatch.getBounds().getMaxY() + BTN_VSPACE), BTN_WIDTH, BTN_HEIGHT));
        add(jIdentify);

        jVerify.setText(BTN_VERIFY);
        jVerify.setToolTipText(MSG_VERIFY);
        jVerify.setBounds(new Rectangle(BTN_LEFT, (int) (jIdentify.getBounds().getMaxY() + BTN_VSPACE), BTN_WIDTH, BTN_HEIGHT));
        add(jVerify);
    }

    /**
     * Adds a listener to be notified when "Verify" button pressed.
     * @param actionListener listener to add.
     */
    public void addVerifyListener(ActionListener actionListener) {
        jVerify.addActionListener(actionListener);
    }

    /**
     * Unsubscribes a listener from "Verify" button events.
     * @param actionListener listener to remove.
     */
    public void removeVerifyListener(ActionListener actionListener) {
        jVerify.removeActionListener(actionListener);
    }

    /**
     * Adds a listener to be notified when "Identify" button pressed.
     * @param actionListener listener to add.
     */
    public void addIdentifyListener(ActionListener actionListener) {
        jIdentify.addActionListener(actionListener);
    }

    /**
     * Unsubscribes a listener from "Identify" button events.
     * @param actionListener listener to remove.
     */
    public void removeIdentifyListener(ActionListener actionListener) {
        jIdentify.removeActionListener(actionListener);
    }

    /**
     * Adds a listener to be notified when "VerifyMatch" button pressed.
     * @param actionListener listener to add.
     */
    public void addVerifyMatchListener(ActionListener actionListener) {
        jVerifyMatch.addActionListener(actionListener);
    }

    /**
     * Unsubscribes a listener from "VerifyMatch" button events.
     * @param actionListener listener to remove.
     */
    public void removeVerifyMatchListener(ActionListener actionListener) {
        jVerifyMatch.removeActionListener(actionListener);
    }

    /**
     * Adds a listener to be notified when "IdentifyMatch" button pressed.
     * @param actionListener listener to add.
     */
    public void addIdentifyMatchListener(ActionListener actionListener) {
        jIdentifyMatch.addActionListener(actionListener);
    }

    /**
     * Unsubscribes a listener from "IdentifyMatch" button events.
     * @param actionListener listener to remove.
     */
    public void removeIdentifyMatchListener(ActionListener actionListener) {
        jIdentifyMatch.removeActionListener(actionListener);
    }

    /**
     * Adds a listener to be notified when "BeginVerifyCapture" button pressed.
     * @param actionListener listener to add.
     */
    public void addBeginVerifyCaptureListener(ActionListener actionListener) {
        jBeginVerifyCapture.addActionListener(actionListener);
    }

    /**
     * Unsubscribes a listener from "BeginVerifyCapture" button events.
     * @param actionListener listener to remove.
     */
    public void removeBeginVerifyCaptureListener(ActionListener actionListener) {
        jBeginVerifyCapture.removeActionListener(actionListener);
    }

    /**
     * Adds a listener to be notified when "EndCapture" button pressed.
     * @param actionListener listener to add.
     */
    public void addEndCaptureListener(ActionListener actionListener) {
        jEndCapture.addActionListener(actionListener);
    }

    /**
     * Unsubscribes a listener from "EndCapture" button events.
     * @param actionListener listener to remove.
     */
    public void removeEndCaptureListener(ActionListener actionListener) {
        jEndCapture.removeActionListener(actionListener);
    }

    /**
     * Adds a listener to be notified when "BeginEnrollCapture" button pressed.
     * @param actionListener listener to add.
     */
    public void addBeginEnrollCaptureListener(ActionListener actionListener) {
        jBeginEnrollCapture.addActionListener(actionListener);
    }

    /**
     * Unsubscribes a listener from "BeginEnrollCapture" button events.
     * @param actionListener listener to remove.
     */
    public void removeBeginEnrollCaptureListener(ActionListener actionListener) {
        jBeginEnrollCapture.removeActionListener(actionListener);
    }

    /**
     * Adds a listener to be notified when "ClearData" button pressed.
     * @param actionListener listener to add.
     */
    public void addClearDataListener(ActionListener actionListener) {
        jClearData.addActionListener(actionListener);
    }

    /**
     * Unsubscribes a listener from "ClearData" button events.
     * @param actionListener listener to remove.
     */
    public void removeClearDataListener(ActionListener actionListener) {
        jClearData.removeActionListener(actionListener);
    }

    /**
     * Adds a listener to be notified when "DeviceDisable" button pressed.
     * @param actionListener listener to add.
     */
    public void addDeviceDisableListener(ActionListener actionListener) {
        jDeviceDisable.addActionListener(actionListener);
    }

    /**
     * Unsubscribes a listener from "DeviceDisable" button events.
     * @param actionListener listener to remove.
     */
    public void removeDeviceDisableListener(ActionListener actionListener) {
        jDeviceDisable.removeActionListener(actionListener);
    }

    /**
     * Adds a listener to be notified when "DeviceEnable" button pressed.
     * @param actionListener listener to add.
     */
    public void addDeviceEnableListener(ActionListener actionListener) {
        jDeviceEnable.addActionListener(actionListener);
    }

    /**
     * Unsubscribes a listener from "DeviceEnable" button events.
     * @param actionListener listener to remove.
     */
    public void removeDeviceEnableListener(ActionListener actionListener) {
        jDeviceEnable.removeActionListener(actionListener);
    }

    /**
     * Adds a listener to be notified when "Release" button pressed.
     * @param actionListener listener to add.
     */
    public void addReleaseListener(ActionListener actionListener) {
        jRelease.addActionListener(actionListener);
    }

    /**
     * Unsubscribes a listener from "Release" button events.
     * @param actionListener listener to remove.
     */
    public void removeReleaseListener(ActionListener actionListener) {
        jRelease.removeActionListener(actionListener);
    }

    /**
     * Adds a listener to be notified when "Claim" button pressed.
     * @param actionListener listener to add.
     */
    public void addClaimListener(ActionListener actionListener) {
        jClaim.addActionListener(actionListener);
    }

    /**
     * Unsubscribes a listener from "Claim" button events.
     * @param actionListener listener to remove.
     */
    public void removeClaimListener(ActionListener actionListener) {
        jClaim.removeActionListener(actionListener);
    }

    /**
     * Adds a listener to be notified when "Close" button pressed.
     * @param actionListener listener to add.
     */
    public void addCloseListener(ActionListener actionListener) {
        jClose.addActionListener(actionListener);
    }

    /**
     * Unsubscribes a listener from "Close" button events.
     * @param actionListener listener to remove.
     */
    public void removeCloseListener(ActionListener actionListener) {
        jClose.removeActionListener(actionListener);
    }

    /**
     * Adds a listener to be notified when "Check Health" button pressed.
     * @param actionListener listener to add.
     */
    public void addCheckHealthListener(ActionListener actionListener) {
        jCheckHealth.addActionListener(actionListener);
    }

    /**
     * Adds a listener to be notified when "Open" button pressed.
     * @param actionListener listener to add.
     */
    public void addOpenListener(ActionListener actionListener) {
        jOpen.addActionListener(actionListener);
    }

    /**
     * Unsubscribes a listener from "Open" button events.
     * @param actionListener listener to remove.
     */
    public void removeOpenListener(ActionListener actionListener) {
        jOpen.removeActionListener(actionListener);
    }

    /**
     * Unsubscribes a listener from "Check Health" button events.
     * @param actionListener listener to remove.
     */
    public void removeCheckHealthListener(ActionListener actionListener) {
        jCheckHealth.removeActionListener(actionListener);
    }

    /**
     * Set the given states of buttons
     *
     * @param enableOpenButton desired state of "Open" button.
     * @param enableCheckHealthButton desired state of "Check Health" button.
     * @param enableCloseButton desired state of "Close" button.
     * @param enableClaimButton desired state of "" button.
     * @param enableReleaseButton desired state of "Release" button.
     * @param enableDeviceEnableButton desired state of "DeviceEnable" button.
     * @param enableDeviceDisableButton desired state of "DeviceDisable" button.
     * @param enableClearDataButton desired state of "ClearData" button.
     * @param enableBeginEnrollCaptureButton desired state of "BeginEnrollCapture" button.
     * @param enableEndCatureButton desired state of "EndCature" button.
     * @param enableBeginVerifyCaptureButton desired state of "BeginVerifyCapture" button.
     * @param enableIdentifyMatchButton desired state of "IdentifyMatch" button.
     * @param enableVerifyMatchButton desired state of "VerifyMatch" button.
     * @param enableIdentifyButton desired state of "Identify" button.
     * @param enableVerifyButton desired state of "Verify" button.
     */
    public void setButtons(
            boolean enableOpenButton, boolean enableCheckHealthButton, boolean enableCloseButton,
            boolean enableClaimButton, boolean enableReleaseButton,
            boolean enableDeviceEnableButton, boolean enableDeviceDisableButton,
            boolean enableClearDataButton,
            boolean enableBeginEnrollCaptureButton, boolean enableEndCatureButton, boolean enableBeginVerifyCaptureButton,
            boolean enableIdentifyMatchButton, boolean enableVerifyMatchButton,
            boolean enableIdentifyButton, boolean enableVerifyButton
    ) {
        jOpen.setEnabled(enableOpenButton);
        jCheckHealth.setEnabled(enableCheckHealthButton);
        jClose.setEnabled(enableCloseButton);
        jClaim.setEnabled(enableClaimButton);
        jRelease.setEnabled(enableReleaseButton);
        jDeviceEnable.setEnabled(enableDeviceEnableButton);
        jDeviceDisable.setEnabled(enableDeviceDisableButton);
        jClearData.setEnabled(enableClearDataButton);
        jBeginEnrollCapture.setEnabled(enableBeginEnrollCaptureButton);
        jEndCapture.setEnabled(enableEndCatureButton);
        jBeginVerifyCapture.setEnabled(enableBeginVerifyCaptureButton);
        jIdentifyMatch.setEnabled(enableIdentifyMatchButton);
        jVerifyMatch.setEnabled(enableVerifyMatchButton);
        jIdentify.setEnabled(enableIdentifyButton);
        jVerify.setEnabled(enableVerifyButton);
    }
}
