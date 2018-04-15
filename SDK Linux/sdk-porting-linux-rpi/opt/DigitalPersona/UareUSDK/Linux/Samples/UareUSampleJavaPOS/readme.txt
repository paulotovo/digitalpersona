================================================================================

                    DigitalPersona U.are.U UPOS for JavaPOS

                     Sample User Interface and Sample Code

                         (c) 2012 DigitalPersona, Inc.

================================================================================

This readme file explains the behavior of the sample UI provided with the U.are.U
UPOS for JavaPOS. The sample UI demonstrates how to use the methods for
fingerprint enrollment and verification included in the header file for the
product.

The application's interface appears at the top left of the screen on displays with
a resolution of 823x652.


===========
MAIN WINDOW
===========

The main window consists of the following areas:

* Buttons area

  This area is located at the left of the window and contains buttons that
  initiate calls to various methods for performing fingerprint enrollment and
  verification operations.

* Messages area

  This area is located above the Buttons area and displays messages that inform
  the user of system activity, invite the user to perform actions such as touching
  the fingerprint reader, or advise the user of system errors. The message that
  appears when you start the application is "Click to open the device...".

* New Image area

  This area is located at the bottom left and displays raw sensor data when a StatusUpdate event is returned 
  signaling raw data is available.

  * Log area

  This area is located in the middle of the window and displays a log of system
  activity, including method calls and error codes.


* Properties area

  This area is located at the right of the window and displays a list of
  properties, both common and specific (in the Property name column), and
  their current values (in the Value column).



Following is a list of the buttons located in the Buttons area and their
functionalities:


1. Open
-------

When this button is clicked, the open method of the Device Control object is
called.

If the call succeeds, the connection with the fingerprint reader is opened and
various properties (common and specific). These properties are set to their
default values, and "Device opened..." appears in the Messages area.

If the method call fails, a failure message appears in the Messages area, and
error codes are displayed in the Log area.


2. Close
--------

When this button is clicked, the close method of the Device Control is called.

If the method call succeeds, the connection with the fingerprint reader is
closed, all of the controls other than the Open button are disabled, and the
properties are reset, or cleared.

If the method call fails, a failure message appears in the Messages area, and
error codes are displayed in the Log area.


3. Claim

--------
Once the connection with the fingerprint reader has been opened, it must be
claimed.

When this button is clicked, the claim method of the Device Control is called, the
claimed property is set to true, and "Exclusive accessed" appears in the Messages
area.

If the method call fails, a failure message appears in the Messages area, and
error codes are displayed in the Log area.


4. Release
----------

When this button is clicked, the release method of the Device Control is called
and "Released exclusive access to the device" appears in the Messages area.

If the method call fails, a failure message appears in the Messages area, and
error codes are displayed in the Log area.


5. Device Enable
----------------

Once the connection with the fingerprint reader has been claimed, it must be
enabled.

When this button is clicked, the deviceEnabled property is set to true and
"Physical Device Operational" appears in the Messages area.


6. Device Disable
-----------------

When this button is clicked, the deviceEnabled property is set to false and
"Disabled Physical Device" appears in the Messages area.


7. Click Clear Data
-------------------

When this button is clicked, the clearInput method of the Device Control is called
and "Clear data to start enrolling again" appears in the Messages are.

If the method call succeeds, the enrollment template array set and the
verification template are cleared. A new verification template and a set of
enrollment templates can now be created.

If the method call fails, a failure message appears in the Messages area, and
error codes are displayed in the Log area.


8. Begin Enroll Capture
-----------------------

Enrollment of fingerprint templates is initiated with this button. Enrolling a
fingerprint consists of capturing four fingerprint images, converting them into
fingerprint pre-enrollment templates, and then creating an enrollment template
from these templates.

When this button is clicked, the beginEnrollCapture method of Device Control is
called and "Touch the sensor four times" appears in the Messages area.

Currently, four pre-enrollment samples must be provided to create an enrollment
template. User-friendly messages are displayed in the Messages area that guide the
user to touch the fingerprint reader four times.

If the method call succeeds, an enrollment template is created and "Total
enrollment completed: N" appears in the Messages area, where N is the total
number of enrollments.

If the method call fails, a failure message appears in the box in the Messages
area. If an error occurs, appropriate messages appear in the Messages area, and
error codes are displayed in the Log area.


9. End Capture
--------------

When the Begin Enroll Capture, Begin Verify Capture, Identify, and Verify buttons
are clicked, this button is enabled until the fingerprint capture is completed.

If this button is clicked while it is enabled, the endCapture method of the Device
Control is called and "End Capture: Biometrics capture is terminated" appears in
the Messages area.


10. Begin Verify Capture
------------------------

When this button is clicked, the beginVerifyCapture method of the Device Control
is called and "Touch the sensor to capture sample data" appears in the Messages
area.

The user touches the fingerprint reader.

If the method call succeeds, a verification template is created and "Sample Data
Captured" appears in the Messages area.

If the method call fails, a failure message appears in the Messages area. If an
error occurs, appropriate messages appear in the Messages area, and error codes
are displayed in the Log area.


11. Identify Match
------------------

When this button is clicked, the identifyMatch method of the Device Control is
called.

If the method call succeeds, a match is performed using all of the enrollment
templates available and the verification template that was created by
clicking the Begin Verify Capture button. A candidate ranking is generated by
listing only the indices of the enrollment templates that match, and
"Identification success!" or "Identification Failed!" appears in the Messages
area.

If the method call fails, a failure message appears in the Messages area. If an
error occurs, appropriate messages appear in the Messages area, and error codes
are displayed in the Log area.


12. Verify Match
----------------

When this button is clicked, the verifyMatch method of the Device Control is
called.

If the method call succeeds, a match is performed using the latest enrollment
template available and the verification template that was created by clicking
the Begin Verify Capture button, and "Verification success!" or "Verification
failed!" appears in the Messages area.

If the method call fails, a failure message appears in the Messages area. If an
error occurs, appropriate messages appear in the Messages area, and error codes
are displayed in the Log area.


13. Identify
------------

When this button is clicked, the identify method of the Device Control is called
and "Please touch the sensor for Identification" appears in the Messages area.

The user touches the fingerprint reader.

If the method call succeeds, a verification template is created on-the-fly. Then
a match is performed using all of the enrollment templates available and the
verification template. A candidate ranking is generated by listing only the
indices of the enrollment templates that match, and "Identification success!" or
"Identification Failed!" appears in the Messages area.

If the method call fails, a failure message appears in the Messages area. If an
error occurs, appropriate messages appear in the Messages area, and error codes
are displayed in the Log area.


14. Verify
----------

When this button is clicked, the verify method of the Device Control is called and
"Please touch the sensor for verification" appears in the Messages area.

The user touches the fingerprint reader.

If the method call succeeds, a verification template is created on-the-fly. Then
a match is performed using the latest enrollment template available and the
verification template, and "Verification success!" or "Verification failed!"
appears in the Messages area.

If the method call fails, a failure message appears in the Messages area. If an
error occurs, appropriate messages appear in the Messages area, and error codes
are displayed in the Log area.

