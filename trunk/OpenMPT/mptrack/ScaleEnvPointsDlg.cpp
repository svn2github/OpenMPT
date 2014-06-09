/*
 * ScaleEnvPointsDlg.cpp
 * ---------------------
 * Purpose: Dialog for scaling instrument envelope points on x and y axis.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "resource.h"
#include "ScaleEnvPointsDlg.h"
#include "ModInstrument.h"


OPENMPT_NAMESPACE_BEGIN


float CScaleEnvPointsDlg::m_fFactorX = 1.0f;
float CScaleEnvPointsDlg::m_fFactorY = 1.0f;


BOOL CScaleEnvPointsDlg::OnInitDialog()
//-------------------------------------
{
	CDialog::OnInitDialog();

	char buffer[10];
	_snprintf(buffer, 9, "%g", m_fFactorX);
	SetDlgItemText(IDC_EDIT_FACTORX, buffer);
	_snprintf(buffer, 9, "%g", m_fFactorY);
	SetDlgItemText(IDC_EDIT_FACTORY, buffer);
	GetDlgItem(IDC_EDIT_FACTORX)->SetFocus();

	return FALSE;	// return TRUE unless you set the focus to a control
}


void CScaleEnvPointsDlg::OnOK()
//-----------------------------
{
	char buffer[10];
	GetDlgItemText(IDC_EDIT_FACTORX, buffer, 9);
	m_fFactorX = ConvertStrTo<float>(buffer);

	GetDlgItemText(IDC_EDIT_FACTORY, buffer, 9);
	m_fFactorY = ConvertStrTo<float>(buffer);

	if(m_fFactorX > 0 && m_fFactorX != 1)
	{
		for(uint32 i = 0; i < m_Env.nNodes; i++)
		{
			m_Env.Ticks[i] = static_cast<WORD>(m_fFactorX * m_Env.Ticks[i]);

			// Checking that the order of points is preserved.
			if(i > 0 && m_Env.Ticks[i] <= m_Env.Ticks[i - 1])
				m_Env.Ticks[i] = m_Env.Ticks[i - 1] + 1;
		}
	}

	if(m_fFactorY != 1)
	{
		for(uint32 i = 0; i < m_Env.nNodes; i++)
		{
			m_Env.Values[i] = Clamp(static_cast<uint8>((m_fFactorY * ((int)m_Env.Values[i] - m_nCenter)) + m_nCenter), uint8(ENVELOPE_MIN), uint8(ENVELOPE_MAX));
		}
	}

	CDialog::OnOK();
}


OPENMPT_NAMESPACE_END
