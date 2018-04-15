import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;

import com.digitalpersona.uareu.*;

public class Identification 
	extends JPanel
	implements ActionListener
{
	private static final long serialVersionUID = 6;
	private static final String ACT_BACK = "back";
	
	private CaptureThread m_capture;
	private Reader  m_reader;
	private JDialog m_dlgParent;
	private JTextArea m_text;
	
	private final String m_strPromptFormat = "    put %s on the reader\n\n";
	private final int m_nFingerCnt = 4; //how many fingerprints to collect for the identification
	private String[]  m_vFingerNames;   //finger names for the collection of prints
	private Fmd[]     m_fmds;
	private final String m_strPrompt1 = "Identification started,\n";

	private Identification(Reader reader){
		m_reader = reader;
		
		m_vFingerNames = new String[m_nFingerCnt + 1]; //one more to accomodate the last collected fingerprint to be identified 
		m_vFingerNames[0] = "your thumb";
		m_vFingerNames[1] = "your index finger";
		m_vFingerNames[2] = "your middle finger";
		m_vFingerNames[3] = "your ring finger";
		m_vFingerNames[4] = "any finger for identification";
		
		m_fmds = new Fmd[m_nFingerCnt];		
	
		final int vgap = 5;
		final int width = 380;
		
		BoxLayout layout = new BoxLayout(this, BoxLayout.Y_AXIS);
		setLayout(layout);
		
		m_text = new JTextArea(22, 1);
		m_text.setEditable(false);
		JScrollPane paneReader = new JScrollPane(m_text);
		add(paneReader);
		Dimension dm = paneReader.getPreferredSize();
		dm.width = width;
		paneReader.setPreferredSize(dm);
		
		add(Box.createVerticalStrut(vgap));
		
		JButton btnBack = new JButton("Back");
		btnBack.setActionCommand(ACT_BACK);
		btnBack.addActionListener(this);
		add(btnBack);
		add(Box.createVerticalStrut(vgap));
	
		setOpaque(true);
	}
	
	public void actionPerformed(ActionEvent e){
		if(e.getActionCommand().equals(ACT_BACK)){
			//cancel capture
			StopCaptureThread();
		}
		else if(e.getActionCommand().equals(CaptureThread.ACT_CAPTURE)){
			//process result
			CaptureThread.CaptureEvent evt = (CaptureThread.CaptureEvent)e;
			if(ProcessCaptureResult(evt)){
				//restart capture thread
				WaitForCaptureThread();
				StartCaptureThread();
			}
			else{
				//destroy dialog
				m_dlgParent.setVisible(false);
			}
		}
	}
	
	private void StartCaptureThread(){
		m_capture = new CaptureThread(m_reader, false, Fid.Format.ANSI_381_2004, Reader.ImageProcessing.IMG_PROC_DEFAULT);
		m_capture.start(this);
	}
	
	private void StopCaptureThread(){
		if(null != m_capture) m_capture.cancel();
	}
	
	private void WaitForCaptureThread(){
		if(null != m_capture) m_capture.join(1000);
	}
	
	private boolean ProcessCaptureResult(CaptureThread.CaptureEvent evt){
		boolean bCanceled = false;
		
		if(null != evt.capture_result){
			if(null != evt.capture_result.image && Reader.CaptureQuality.GOOD == evt.capture_result.quality){
				//which finger?
				int nIdx = 0;
				for(nIdx = 0; nIdx < m_nFingerCnt; nIdx++){
					if(null == m_fmds[nIdx]) break;
				}
					
				//extract features
				Fmd fmdToIdentify = null;
				Engine engine = UareUGlobal.GetEngine();
				try{
					if(m_nFingerCnt > nIdx) m_fmds[nIdx] = engine.CreateFmd(evt.capture_result.image, Fmd.Format.ANSI_378_2004);
					else fmdToIdentify = engine.CreateFmd(evt.capture_result.image, Fmd.Format.ANSI_378_2004);
				}
				catch(UareUException e){ MessageBox.DpError("Engine.CreateFmd()", e); }
					
				if(m_nFingerCnt == nIdx){
					//collected 5 fingerprints
					try{
						//target false positive identification rate: 0.00001
						//for a discussion of  how to evaluate dissimilarity scores, as well as the statistical validity of the dissimilarity score and error rates, consult the Developer Guide
						int falsepositive_rate = Engine.PROBABILITY_ONE / 100000; 
						
						Engine.Candidate[] vCandidates = engine.Identify(fmdToIdentify, 0, m_fmds, falsepositive_rate, m_nFingerCnt);
							
						if(0 != vCandidates.length){
							//optional: to get false match rate compare with the top candidate
							int falsematch_rate = engine.Compare(fmdToIdentify, 0, m_fmds[vCandidates[0].fmd_index], vCandidates[0].view_index);
								
							String str = String.format("Fingerprint identified, %s\n", m_vFingerNames[vCandidates[0].fmd_index]);
							m_text.append(str);
							str = String.format("dissimilarity score: 0x%x.\n", falsematch_rate);
							m_text.append(str);
							str = String.format("false match rate: %e.\n\n\n", (double)(falsematch_rate / Engine.PROBABILITY_ONE));
							m_text.append(str);
						}
						else{
							m_text.append("Fingerprint was not identified.\n\n\n");
						}
					} catch(UareUException e){ MessageBox.DpError("Engine.Identify()", e); }
						
					//discard FMDs
					for(int i = 0; i < m_nFingerCnt; i++) m_fmds[i] = null;
						
					//prompt for the next loop
					m_text.append(m_strPrompt1);
					String str = String.format(m_strPromptFormat, m_vFingerNames[0]);
					m_text.append(str);
				}
				else{
					//prompt for the next finger
					String str = String.format(m_strPromptFormat, m_vFingerNames[nIdx + 1]);
					m_text.append(str);
				}
			}
			else if(Reader.CaptureQuality.CANCELED == evt.capture_result.quality){
				//capture or streaming was canceled, just quit
				bCanceled = true;
			}
			else{
				//bad quality
				MessageBox.BadQuality(evt.capture_result.quality);
			}
		}
		else if(null != evt.exception){
			//exception during capture
			MessageBox.DpError("Capture", evt.exception);
			bCanceled = true;
		}
		else if(null != evt.reader_status){
			//reader failure
			MessageBox.BadStatus(evt.reader_status);
			bCanceled = true;
		}

		return !bCanceled;
	}
	
	private void doModal(JDialog dlgParent){
		//open reader
		try{
			m_reader.Open(Reader.Priority.COOPERATIVE);
		}
		catch(UareUException e){ MessageBox.DpError("Reader.Open()", e); }
		
		//start capture thread
		StartCaptureThread();

		//put initial prompt on the screen
		m_text.append(m_strPrompt1);
		String str = String.format(m_strPromptFormat, m_vFingerNames[0]);
		m_text.append(str);
		
		//bring up modal dialog
		m_dlgParent = dlgParent;
		m_dlgParent.setContentPane(this);
		m_dlgParent.pack();
		m_dlgParent.setLocationRelativeTo(null);
		m_dlgParent.toFront();
		m_dlgParent.setVisible(true);
		m_dlgParent.dispose();
		
		//cancel capture
		StopCaptureThread();
		
		//wait for capture thread to finish
		WaitForCaptureThread();
		
		//close reader
		try{
			m_reader.Close();
		}
		catch(UareUException e){ MessageBox.DpError("Reader.Close()", e); }
	}
	
	public static void Run(Reader reader){
		JDialog dlg = new JDialog((JDialog)null, "Identification", true);
		Identification identification = new Identification(reader);
		identification.doModal(dlg);
	}
}
