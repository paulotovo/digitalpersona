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

public class Capabilities 
	extends JPanel
	implements ActionListener
{
	private static final long serialVersionUID = 3;
	private static final String ACT_BACK = "back";
	
	private JDialog m_dlgParent;
	
	private Capabilities(Reader reader){
		final int vgap = 5;
		final int width = 380;
		
		BoxLayout layout = new BoxLayout(this, BoxLayout.Y_AXIS);
		setLayout(layout);
		
		JTextArea textReader = new JTextArea(22, 1);
		textReader.setEditable(false);
		JScrollPane paneReader = new JScrollPane(textReader);
		add(paneReader);
		Dimension dmSize = paneReader.getPreferredSize();
		dmSize.width = width;
		paneReader.setPreferredSize(dmSize);
		
		add(Box.createVerticalStrut(vgap));
		
		JButton btnBack = new JButton("Back");
		btnBack.setActionCommand(ACT_BACK);
		btnBack.addActionListener(this);
		add(btnBack);
		add(Box.createVerticalStrut(vgap));

		setOpaque(true);
		
		//print out description
		Reader.Description rd = reader.GetDescription();
		String str = String.format("Vendor name: %s\n", rd.id.vendor_name);
		textReader.append(str);
		str = String.format("Product name: %s\n", rd.id.product_name);
		textReader.append(str);
		str = String.format("Serial number: %s\n", rd.serial_number);
		textReader.append(str);
		str = String.format("USB VID: %s\n", rd.id.vendor_id);
		textReader.append(str);
		str = String.format("USB PID: %s\n", rd.id.product_id);
		textReader.append(str);
		str = String.format("USB BCD revision: %s\n", rd.version.bcd_revision);
		textReader.append(str);
		str = String.format("HW version: %d.%d.%d\n", rd.version.hardware_version.major, rd.version.hardware_version.minor, rd.version.hardware_version.maintenance);
		textReader.append(str);
		str = String.format("FW version: %d.%d.%d\n", rd.version.firmware_version.major, rd.version.firmware_version.minor, rd.version.firmware_version.maintenance);
		textReader.append(str);

		try{
			//open reader
			reader.Open(Reader.Priority.COOPERATIVE);
			
			//acquire capabilities
			Reader.Capabilities rc = reader.GetCapabilities();
			
			//close reader
			reader.Close();
			
			//print out capabilities
			textReader.append("\n");
			str = String.format("can capture image: %b\n", rc.can_capture);
			textReader.append(str);
			str = String.format("can stream image: %b\n", rc.can_stream);
			textReader.append(str);
			str = String.format("can extract features: %b\n", rc.can_extract_features);
			textReader.append(str);
			str = String.format("can match: %b\n", rc.can_match);
			textReader.append(str);
			str = String.format("can identify: %b\n", rc.can_identify);
			textReader.append(str);
			str = String.format("has fingerprint storage: %b\n", rc.has_fingerprint_storage);
			textReader.append(str);
			str = String.format("indicator type: %d\n", rc.indicator_type);
			textReader.append(str);
			str = String.format("has power management: %b\n", rc.has_power_management);
			textReader.append(str);
			str = String.format("has calibration: %b\n", rc.has_calibration);
			textReader.append(str);
			str = String.format("PIV compliant: %b\n", rc.piv_compliant);
			textReader.append(str);
			for(int i = 0; i < rc.resolutions.length; i++){
				str = String.format("resolution: %d dpi\n", rc.resolutions[i]);
				textReader.append(str);
			}
		} 
		catch(UareUException e){ 
			MessageBox.DpError("Reader.GetCapabilities()", e); 
		}
		
		
	}
	
	public void actionPerformed(ActionEvent e){
		if(e.getActionCommand().equals(ACT_BACK)){
			m_dlgParent.setVisible(false);
		}
	}

	private void doModal(JDialog dlgParent){
		m_dlgParent = dlgParent;
		m_dlgParent.setContentPane(this);
		m_dlgParent.pack();
		m_dlgParent.setLocationRelativeTo(null);
		m_dlgParent.toFront();
		m_dlgParent.setVisible(true);
		m_dlgParent.dispose();
	}
	
	public static void Show(Reader reader){
    	JDialog dlg = new JDialog((JDialog)null, "Reader capabilities", true);
    	Capabilities capabilities = new Capabilities(reader);
    	capabilities.doModal(dlg);
	}
}
