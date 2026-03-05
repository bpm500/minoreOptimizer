// PropertyListCtrl.cpp : implementation file
//
// created by Unwinder
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "resource.h"
#include "PropertyListCtrl.h"
/////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
/////////////////////////////////////////////////////////////////////////////
// CPropertyListCtrl
/////////////////////////////////////////////////////////////////////////////
CPropertyListCtrl::CPropertyListCtrl()
{
	m_bAllowEmptyRootExpand			= TRUE;
	m_bAllowItemSelection			= TRUE;
	m_bDrawRootButtons				= FALSE;
	m_bDrawPropertyCells			= TRUE;
	m_bEnableMultiSelection			= FALSE;
	m_bEnableItemEdit				= FALSE;

	m_bHeaderDragging				= FALSE;

	m_iSelectedItem					= -1;
}
/////////////////////////////////////////////////////////////////////////////
CPropertyListCtrl::~CPropertyListCtrl()
{
}
/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CPropertyListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CPropertyListCtrl)
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	ON_WM_GETDLGCODE()
	//}}AFX_MSG_MAP
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnclick)
	ON_NOTIFY_REFLECT(LVN_ENDLABELEDIT, OnEndlabeledit)
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// CPropertyListCtrl message handlers
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	if (lpDrawItemStruct->CtlType != ODT_LISTVIEW)
        return;
	
	if(lpDrawItemStruct->itemAction == ODA_DRAWENTIRE)
	{
		int		iItem		= lpDrawItemStruct->itemID;
		CDC*	pDC			= CDC::FromHandle(lpDrawItemStruct->hDC);
		CRect	itemRect	= lpDrawItemStruct->rcItem;

		DrawItem(iItem, lpDrawItemStruct->itemState, pDC, itemRect);
	}
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::DrawItem(int iItem, DWORD itemState, CDC *pDC, CRect itemRect, BOOL /*bDrawAlways*/)
{
	PLC_ITEM_DESC* pItemDesc = GetItemDesc(iItem);

	if (!IsValidItemDesc(pItemDesc))
		return;

	CRect boundsRect; 
	GetItemRect(iItem, boundsRect, LVIR_BOUNDS);

	CRect labelRect;
	GetItemRect(iItem, labelRect, LVIR_LABEL);

	CRect iconRect;
	GetItemRect(iItem, iconRect, LVIR_ICON);

	iconRect.right = iconRect.left + iconRect.Height();

	CRect clientRect; 
	GetClientRect(clientRect);

	int iColumns = 0;

	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	if (pHeaderCtrl)
		iColumns = pHeaderCtrl->GetItemCount();

	if (!iColumns)
		return;

	int* cw = new int[iColumns];
	int  tw = 0;

	for (int iColumn=0; iColumn<iColumns; iColumn++)
	{
		cw[iColumn]  = GetColumnWidth(iColumn);
		tw			+= cw[iColumn];
	}

	PrepareMemDC(pDC);

	CPen* pOldPen = m_memDC.GetCurrentPen();
		
	COLORREF cellBorderColor = GetSysColor (COLOR_WINDOW);
	COLORREF cellColor0 = RGB(GetRValue(cellBorderColor)*0.90f,
							  GetGValue(cellBorderColor)*0.90f,
							  GetBValue(cellBorderColor)*0.90f);
	COLORREF cellColor1 = RGB(GetRValue(cellBorderColor)*0.80f,
							  GetGValue(cellBorderColor)*0.80f,
							  GetBValue(cellBorderColor)*0.80f);

	CRect cellRect	= boundsRect;
	cellRect.right  -= 1; 
	cellRect.bottom -= 1;

	CBrush cellBrush0(cellColor0);
	CBrush cellBrush1(cellColor1);

	//draw cell

	if (m_bDrawPropertyCells)
	{
		if (pItemDesc->dwFlags & PLC_ITEM_FLAG_ROOT)
		{
			if (pItemDesc->dwFlags & PLC_ITEM_FLAG_PROPERTY_COLUMN)
			{
				CRect cellRect0   = cellRect;
				cellRect0.right  -= tw - cw[0]; 

				m_memDC.FillRect(cellRect0, &cellBrush1);

				for (int iColumn=1; iColumn<iColumns; iColumn++)
				{
					cellRect0.right  += cw[iColumn]; 
					cellRect0.left   += cw[iColumn - 1];

					m_memDC.FillRect(cellRect0, &cellBrush1);
				}
			}
			else
				m_memDC.FillRect(cellRect, &cellBrush1);
		}
		else
		{
			if (pItemDesc->dwFlags & PLC_ITEM_FLAG_PROPERTY_COLUMN)
			{
				CRect cellRect0   = cellRect;
				cellRect0.right  -= tw - cw[0]; 

				m_memDC.FillRect(cellRect0, &cellBrush0);

				for (int iColumn=1; iColumn<iColumns; iColumn++)
				{
					cellRect0.right  += cw[iColumn]; 
					cellRect0.left   += cw[iColumn - 1];

					m_memDC.FillRect(cellRect0, &cellBrush1);
				}
			}
			else
				m_memDC.FillRect(cellRect, &cellBrush0);
		}
	}
	else
		m_memDC.FillRect(cellRect, &cellBrush0);

	//draw cell border

    CPen cellBorderPen(PS_SOLID, 1, cellBorderColor);
	m_memDC.SelectObject(&cellBorderPen);

	m_memDC.MoveTo(cellRect.left , cellRect.bottom );
	m_memDC.LineTo(cellRect.right, cellRect.bottom );
	m_memDC.LineTo(cellRect.right, cellRect.top - 1);

	//draw property separator

	if (m_bDrawPropertyCells)
	{
		if (pItemDesc->dwFlags & PLC_ITEM_FLAG_PROPERTY_COLUMN)
		{
			int x = 0;

			for (int iColumn=iColumns-1; iColumn>0; iColumn--)
			{
				x += cw[iColumn];
			
				m_memDC.MoveTo(boundsRect.right - x - 1 , boundsRect.bottom );
				m_memDC.LineTo(boundsRect.right - x - 1 , boundsRect.top - 1);
			}

			m_memDC.SelectObject(pOldPen);
		}
	}

	m_memDC.SelectObject(pOldPen);

	COLORREF bgndColor, textColor;
		
	if (itemState & ODS_FOCUS)
	{
		bgndColor = GetSysColor (COLOR_HIGHLIGHT);
		textColor = GetSysColor (COLOR_HIGHLIGHTTEXT);
	}
	else
	if (m_bEnableMultiSelection && (itemState & ODS_SELECTED))
	{
		bgndColor = Blend(GetSysColor (COLOR_HIGHLIGHT), GetSysColor(COLOR_WINDOW), 0.3f);
		textColor = GetSysColor(COLOR_HIGHLIGHTTEXT);
	}
	else
	{
		bgndColor = GetSysColor (COLOR_WINDOW);

		if (pItemDesc->dwFlags & PLC_ITEM_FLAG_HOT)
			textColor = GetSysColor (COLOR_HOTLIGHT);
		else
			textColor = GetSysColor (COLOR_WINDOWTEXT);
	}

	if (pItemDesc->dwFlags & PLC_ITEM_FLAG_PROPERTY_COLUMN)
	{
		itemRect.right = labelRect.right;

		if (m_bDrawPropertyCells)
			itemRect.right--;
	}
	else
		itemRect.right--;

	CBrush bgndBrush(bgndColor);

	//draw focus rect

	if ((itemState & ODS_FOCUS) || (m_bEnableMultiSelection && (itemState & ODS_SELECTED)))
		m_memDC.FillRect(itemRect, &bgndBrush);

	//draw root buttons and checkmarks

	if (pItemDesc->dwFlags & PLC_ITEM_FLAG_ROOT)
	{
		if (m_bAllowEmptyRootExpand || (pItemDesc->dwFlags & PLC_ITEM_FLAG_ALLOW_ROOT_EXPAND))
			DrawRootButton(&m_memDC, iconRect, pItemDesc->dwFlags & PLC_ITEM_FLAG_EXPANDED);
	}
	else
	{
		if (m_bAllowItemSelection)
			DrawCheck(&m_memDC, iconRect, pItemDesc->dwFlags & PLC_ITEM_FLAG_SELECTED);
	}

	//draw text

	if (pItemDesc->dwFlags & PLC_ITEM_FLAG_ROOT)
	{
		if (m_bDrawRootButtons || m_bAllowEmptyRootExpand || (pItemDesc->dwFlags & PLC_ITEM_FLAG_ALLOW_ROOT_EXPAND))
			itemRect.left  += 16;
	}
	else
	{
		if (m_bAllowItemSelection)
			itemRect.left  += 16;
	}

	LPCTSTR pszText = MakeShortString(&m_memDC, pItemDesc->szText, itemRect.Width(), 4);
	
	if (pItemDesc->dwFlags & PLC_ITEM_FLAG_ROOT)
	{
		if (m_bDrawRootButtons || m_bAllowEmptyRootExpand || (pItemDesc->dwFlags & PLC_ITEM_FLAG_ALLOW_ROOT_EXPAND))
			itemRect.left  += 10;
		else
			itemRect.left  += 4;
	}
	else
	{
		if (m_bAllowItemSelection)
			itemRect.left  += 10;
		else
			itemRect.left  += 4;
	}
	itemRect.right -= 2;
		
	m_memDC.SetBkColor  (bgndColor);
	m_memDC.SetTextColor(textColor);
	m_memDC.SetBkMode(TRANSPARENT);

	m_memDC.DrawText(pszText,-1, itemRect,DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER | DT_EXTERNALLEADING);

	//draw property text

	if (pItemDesc->dwFlags & PLC_ITEM_FLAG_PROPERTY_COLUMN)
	{
		for (int iColumn=1; iColumn<iColumns; iColumn++)
		{
			CRect subItemRect; GetSubItemRect(iItem, iColumn, LVIR_LABEL, subItemRect);

			if ((itemState & ODS_FOCUS) || (m_bEnableMultiSelection && (itemState & ODS_SELECTED)))
			{
				CRect focusRect = subItemRect; 
				focusRect.right--;

				m_memDC.FillRect(focusRect, &bgndBrush);
			}

			char szSubitemText[MAX_PATH];
			GetItemText(iItem, iColumn, szSubitemText, MAX_PATH);

			if (pItemDesc->dwFlags & PLC_ITEM_FLAG_PROPERTY_TEXT_CALLBACK)
			{
				CString strPropertyText = GetPropertyTextCallback(iItem, iColumn);
			
				pszText = MakeShortString(&m_memDC, strPropertyText, subItemRect.Width(), 4);
			}
			else
				pszText = MakeShortString(&m_memDC, (iColumn == 1) ? pItemDesc->szPropertyText : szSubitemText, subItemRect.Width(), 4);

			subItemRect.left  += 2;
			subItemRect.right -= 2;

			m_memDC.DrawText(pszText,-1, subItemRect, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER | DT_EXTERNALLEADING);
		}
	}

	delete [] cw;

	pDC->BitBlt(boundsRect.left, boundsRect.top, boundsRect.Width(), boundsRect.Height(), &m_memDC,boundsRect.left, boundsRect.top,SRCCOPY);
}
/////////////////////////////////////////////////////////////////////////////
PLC_ITEM_DESC* CPropertyListCtrl::InsertItem(DWORD dwFlags, LPCSTR lpszText, LPCSTR lpszPropertyText, PLC_ITEM_DESC *pRoot, DWORD dwUserData0, DWORD dwUserData1, DWORD dwUserData2, DWORD dwUserData3)
{
	PLC_ITEM_DESC* pDesc = new PLC_ITEM_DESC; 
	ZeroMemory(pDesc, sizeof(PLC_ITEM_DESC));

	pDesc->dwFlags		= dwFlags;

	pDesc->dwUserData0	= dwUserData0;
	pDesc->dwUserData1	= dwUserData1;
	pDesc->dwUserData2	= dwUserData2;
	pDesc->dwUserData3	= dwUserData3;

	if (pRoot)
	{
		pDesc->pRoot	= pRoot;
		pRoot->dwFlags |= PLC_ITEM_FLAG_ALLOW_ROOT_EXPAND;
	}
	else
		pDesc->dwFlags |= PLC_ITEM_FLAG_ROOT | PLC_ITEM_FLAG_LAST_CHILD;


	if (!(dwFlags & PLC_ITEM_FLAG_TEXT_CALLBACK))
	{
		if (!lpszText)
			return NULL;
		else
			strcpy_s(pDesc->szText, sizeof(pDesc->szText), lpszText);
	}

	if (!(dwFlags & PLC_ITEM_FLAG_PROPERTY_TEXT_CALLBACK))
	{
		if (!lpszPropertyText)
			return NULL;
		else
			strcpy_s(pDesc->szPropertyText, sizeof(pDesc->szPropertyText), lpszPropertyText);
	}

	if (pRoot)
	{
		POSITION pos = m_itemsList.GetHeadPosition();

		while (pos)
		{
			PLC_ITEM_DESC* pCurrentDesc = m_itemsList.GetAt(pos);

			if ((pCurrentDesc == pRoot) || (pCurrentDesc->pRoot == pRoot)) 
			{
				if (pCurrentDesc->dwFlags & PLC_ITEM_FLAG_LAST_CHILD)
				{
					pCurrentDesc->dwFlags &= ~PLC_ITEM_FLAG_LAST_CHILD;
					pDesc->dwFlags |= PLC_ITEM_FLAG_LAST_CHILD;

					m_itemsList.InsertAfter(pos, pDesc);
					break;
				}
			}

			m_itemsList.GetNext(pos);
		}
		
		if (!pos)
			m_itemsList.AddTail(pDesc);

	}
	else
		m_itemsList.AddTail(pDesc);

	return pDesc;
}
/////////////////////////////////////////////////////////////////////////////
PLC_ITEM_DESC* CPropertyListCtrl::GetItemDesc(int iItem)
{
	if (!IsValidItem(iItem))
		return NULL;

	return (PLC_ITEM_DESC*)GetItemData(iItem);
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::PrepareMemDC(CDC *pDC)
{
	CRect clientRect; GetClientRect(&clientRect);

	if (m_memDC.GetSafeHdc())
		return;

	m_memDC.CreateCompatibleDC(pDC);
	m_bitmap.CreateCompatibleBitmap(pDC, clientRect.Width(), clientRect.Height());
	m_memDC.SelectObject(&m_bitmap);

	m_memDC.SelectObject(pDC->GetCurrentFont());
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::DestroyMemDC()
{
	if (m_memDC.GetSafeHdc())
		VERIFY(m_memDC.DeleteDC());
	if (m_bitmap.GetSafeHandle())
		VERIFY(m_bitmap.DeleteObject());
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::OnDestroy() 
{
	Uninit();
	DestroyMemDC();

	CListCtrl::OnDestroy();

	m_imageList.DeleteImageList();
}
/////////////////////////////////////////////////////////////////////////////
LPCTSTR CPropertyListCtrl::MakeShortString(CDC *pDC, LPCTSTR lpszLong, int nColumnLen, int nOffset)
{
	static const _TCHAR szThreeDots[]=_T("...");

	int nStringLen=lstrlen(lpszLong);

	if((nStringLen==0) || (pDC->GetTextExtent(lpszLong,nStringLen).cx + nOffset < nColumnLen))
		return(lpszLong);

	static _TCHAR szShort[MAX_PATH]; lstrcpy(szShort,lpszLong);

	int nAddLen = pDC->GetTextExtent(szThreeDots,sizeof(szThreeDots)).cx;

	for(int i=nStringLen-1; i > 0; i--)
	{
		szShort[i]=0;
		if(pDC->GetTextExtent(szShort,i).cx + nOffset + nAddLen < nColumnLen)
			break;
	}

	lstrcat(szShort,szThreeDots);

	return(szShort);
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::DrawRootButton(CDC *pDC, CRect rect, BOOL bExpanded)
{
	if (m_bDrawRootButtons)
	{
		int x		= rect.left + ((16				- 9)>>1) - 1;
		int y		= rect.top  + ((rect.Height()	- 9)>>1) - 1;

		CRect signRect(x, y, x+9, y+9);
		pDC->Rectangle(signRect);
				
		if (bExpanded)
		{
			pDC->MoveTo(x+2, y+4);
			pDC->LineTo(x+7, y+4);
		}
		else
		{
			pDC->MoveTo(x+4, y+2);
			pDC->LineTo(x+4, y+7);
			pDC->MoveTo(x+2, y+4);
			pDC->LineTo(x+7, y+4);
		}
	}
	else
		DrawCheck(pDC, rect, bExpanded);
}
/////////////////////////////////////////////////////////////////////////////
int CPropertyListCtrl::HitTestOnPlus(CPoint point)
{
	int iItem = HitTestOnItem(point);

	if (iItem == -1)
		return -1;

	PLC_ITEM_DESC* pItemDesc = GetItemDesc(iItem);

	if (!IsValidItemDesc(pItemDesc))
		return -1;

	if (pItemDesc->dwFlags & PLC_ITEM_FLAG_ROOT)
	{
		if ((point.x + GetScrollPos(SB_HORZ)) < 16)
			return iItem;
	}

	return -1;
}
/////////////////////////////////////////////////////////////////////////////
int CPropertyListCtrl::HitTestOnItem(CPoint point)
{
	int iItem = HitTest(point);

	CRect viewRect  = GetItemsRect();

	if (!viewRect.PtInRect(point))
		return -1;

	return iItem;
}
/////////////////////////////////////////////////////////////////////////////
CRect CPropertyListCtrl::GetItemsRect()
{
	int iItems = GetItemCount();

	if (iItems)
	{
		CRect firstItemRect; GetItemRect(0, firstItemRect, LVIR_BOUNDS);
		CRect viewRect; GetItemRect(iItems - 1, viewRect, LVIR_BOUNDS);
		viewRect.top  = firstItemRect.top;
		viewRect.left = firstItemRect.left;
		return viewRect;
	}
	else
		return CRect(0,0,0,0);
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	SetFocus();

	int iItem = HitTestOnPlus(point);

	if (iItem == -1)
	{
		iItem = HitTestOnCheck(point);

		if (iItem == -1)
			CListCtrl::OnLButtonDown(nFlags, point);
		else
		{
			if (m_bAllowItemSelection)
			{
				PLC_ITEM_DESC* pItemDesc = GetItemDesc(iItem);

				pItemDesc->dwFlags ^= PLC_ITEM_FLAG_SELECTED;

				Update(iItem);

				OnCheckItem(iItem);
			}

		}
	}
	else
	{
		PLC_ITEM_DESC* pItemDesc = GetItemDesc(iItem);

		if (IsValidItemDesc(pItemDesc))
		{
			if (pItemDesc->dwFlags & PLC_ITEM_FLAG_EXPANDED)
				CollapseItem(iItem);
			else
				ExpandItem(iItem);
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
int CPropertyListCtrl::HitTestOnCheck(CPoint point)
{
	int iItem = HitTestOnItem(point);

	if (iItem == -1)
		return -1;

	PLC_ITEM_DESC* pItemDesc = GetItemDesc(iItem);

	if (!IsValidItemDesc(pItemDesc))
		return -1;

	if (!(pItemDesc->dwFlags & PLC_ITEM_FLAG_ROOT))
		if ((point.x + GetScrollPos(SB_HORZ)) < 16)
			return iItem;

	return -1;
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::CollapseItem(int iItem)
{
	if (!IsValidItem(iItem))
		return;

	PLC_ITEM_DESC* pItemDesc = GetItemDesc(iItem);

	if (!IsValidItemDesc(pItemDesc))
		return;

	if (!(pItemDesc->dwFlags & PLC_ITEM_FLAG_EXPANDED))
		return;

	PLC_ITEM_DESC* pRoot = (pItemDesc->dwFlags & PLC_ITEM_FLAG_ROOT) ? pItemDesc : pItemDesc->pRoot;

	int iItems = GetItemCount();

	if (!iItems)
		return;

	OnCollapseItem(iItem);

	SetRedraw(FALSE);

	BOOL bChangeSelection = FALSE;

	int iSelectedItem = GetSelectionMark();

	if (iSelectedItem != -1)
	{
		pItemDesc = (PLC_ITEM_DESC*)GetItemData(iSelectedItem);

		if (IsValidItemDesc(pItemDesc))
		{
			if (pRoot == pItemDesc->pRoot)
				bChangeSelection = TRUE;
		}
	}
	
	int iNewSelectedItem = 0;

	for (iItem=0; iItem<iItems; iItem++)
	{
		pItemDesc = (PLC_ITEM_DESC*)GetItemData(iItem);

		if (IsValidItemDesc(pItemDesc))
		{
			if (pItemDesc == pRoot)
			{
				if (pItemDesc->dwFlags & PLC_ITEM_FLAG_ROOT)
				{
					pItemDesc->dwFlags &= ~PLC_ITEM_FLAG_EXPANDED;
					iNewSelectedItem = iItem;
					Update(iItem);
				}
			}
			
			if (pRoot == pItemDesc->pRoot)
			{
				{
					DeleteItem(iItem);
					iItem--;
					iItems--;
				}
			}
		}
	}

	if (bChangeSelection)
	{
		SetSelectionMark(iNewSelectedItem);
		SetItemState(iNewSelectedItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
		EnsureVisible(iNewSelectedItem, FALSE);
	}

	SetRedraw(TRUE);
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::ExpandItem(int iItem)
{
	if (!IsValidItem(iItem))
		return;

	PLC_ITEM_DESC* pItemDesc = GetItemDesc(iItem);

	if (!IsValidItemDesc(pItemDesc))
		return;

	if (!(pItemDesc->dwFlags & PLC_ITEM_FLAG_ROOT))
		return;

	if (pItemDesc->dwFlags & PLC_ITEM_FLAG_EXPANDED)
		return;

	OnExpandItem(iItem);

	PLC_ITEM_DESC* pRoot = (pItemDesc->dwFlags & PLC_ITEM_FLAG_ROOT) ? pItemDesc : pItemDesc->pRoot;

	pItemDesc->dwFlags |= PLC_ITEM_FLAG_EXPANDED;

	SetRedraw(FALSE);

	Update(iItem);

	iItem++;

	POSITION pos = m_itemsList.GetHeadPosition();

	while (pos)
	{
		PLC_ITEM_DESC* pSrcDesc = m_itemsList.GetNext(pos);

		if (pSrcDesc->pRoot == pRoot)
		{
			CListCtrl::InsertItem(iItem, "");
			SetItemData(iItem, (DWORD)pSrcDesc);

			iItem++;

		}
	}

	SetRedraw(TRUE);
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::Init()
{
	ListView_SetExtendedListViewStyle(m_hWnd, LVS_EX_DOUBLEBUFFER);

	m_imageList.Create();
	m_imageList.Add(IDI_SELECT);
	m_imageList.Add(IDI_SELECT_PLACEHOLDER);
	SetImageList(&m_imageList, LVSIL_SMALL);

	int index = 0;

	POSITION pos = m_itemsList.GetHeadPosition();

	while (pos)
	{
		PLC_ITEM_DESC* pItemDesc = m_itemsList.GetNext(pos);

		if (pItemDesc->dwFlags & PLC_ITEM_FLAG_ROOT)
			SetItemData(CListCtrl::InsertItem(index++, ""), (DWORD)pItemDesc);

		if (pItemDesc->pRoot)
			if (pItemDesc->pRoot->dwFlags & PLC_ITEM_FLAG_EXPANDED)
				SetItemData(CListCtrl::InsertItem(index++, ""), (DWORD)pItemDesc);
	}
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	if ((HitTestOnPlus(point) != -1) || (HitTestOnCheck(point) != -1))
		OnLButtonDown(nFlags, point);
	else
		OnDblClkItem(HitTestOnItem(point), HitTestOnSubitem(point));
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::DrawCheck(CDC* /*pDC*/, CRect rect, BOOL bChecked)
{
	if (bChecked)
		ImageList_DrawEx(m_imageList.GetSafeHandle(), 0, m_memDC.m_hDC, rect.left - 2, rect.top + ((rect.Height() - 16)>>1), 16, 16, CLR_DEFAULT, CLR_DEFAULT, ILD_TRANSPARENT);
	else
		ImageList_DrawEx(m_imageList.GetSafeHandle(), 1, m_memDC.m_hDC, rect.left - 2, rect.top + ((rect.Height() - 16)>>1), 16, 16, CLR_DEFAULT, CLR_DEFAULT, ILD_TRANSPARENT);
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (!pScrollBar)
	{
		UndoEdit();

		InvalidateBgndRgn();
	}
	
	CListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::UndoEdit()
{
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (!pScrollBar)
	{
		UndoEdit();

		InvalidateBgndRgn();
	}
	
	CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
}
/////////////////////////////////////////////////////////////////////////////
BOOL CPropertyListCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	UndoEdit();

	InvalidateBgndRgn();

	return CListCtrl::OnMouseWheel(nFlags, zDelta, pt);
}
/////////////////////////////////////////////////////////////////////////////
LRESULT CPropertyListCtrl::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if ((message == WM_NOTIFY) && !wParam)
	{
		NMHDR* pNMHDR = (NMHDR*)lParam;

		if (pNMHDR->code == HDN_DIVIDERDBLCLICKW)
			return 0;

		if (pNMHDR->code == HDN_BEGINTRACKW)
		{
			m_bHeaderDragging = TRUE;

			UndoEdit();
		}

		if (pNMHDR->code == HDN_ENDTRACKW)
		{
			m_bHeaderDragging = FALSE;

			RedrawVisibleItems();

			DestroyMemDC();
			InvalidateBgndRgn();
			RedrawVisibleItems();
		}
	}
	
	return CListCtrl::DefWindowProc(message, wParam, lParam);
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::RedrawVisibleItems()
{
	int iItems = GetItemCount();
	int iTopIndex = GetTopIndex();
	int iLastIndex = iTopIndex + GetCountPerPage() + 1;

	if (iLastIndex > iItems)
		iLastIndex = iItems;

	RedrawItems(iTopIndex, iLastIndex);
}
/////////////////////////////////////////////////////////////////////////////
BOOL CPropertyListCtrl::IsEditing()
{
	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
CString CPropertyListCtrl::GetPropertyTextCallback(int /*iItem*/, int /*iColumn*/)
{
	return "";
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::OnCheckItem(int /*iItem*/)
{
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::OnExpandItem(int /*iItem*/)
{
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::OnCollapseItem(int /*iItem*/)
{
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::Uninit()
{
	POSITION pos = m_itemsList.GetHeadPosition();

	while (pos)
		delete m_itemsList.GetNext(pos);

	m_itemsList.RemoveAll();
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::OnDblClkItem(int iItem, int iSubItem)
{
	if (iItem != -1)
		DoEdit(iItem, iSubItem);
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	int iColumns = 0;

	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	
	if (pHeaderCtrl)
		iColumns = pHeaderCtrl->GetItemCount();

	if (iColumns <= 2)
	{
		CRect cr; GetClientRect(&cr);
		int column		= pNMListView->iSubItem;
		int columnWidth = cr.Width() - GetColumnWidth(column ^ 1);

		if (columnWidth > 0)
		{
			SetRedraw(FALSE);

			SetColumnWidth(column, columnWidth);

			DestroyMemDC();

			CRect viewRect = GetItemsRect();
			ValidateRect(&viewRect);

			InvalidateBgndRgn();
			RedrawVisibleItems();

			SetRedraw(TRUE);
		}
	}

	*pResult = 0;
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == ' ')
		ToggleCurSel();

	CListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::ToggleCurSel()
{
	if (!m_bAllowItemSelection)
		return;

	int iItem = GetSelectionMark();

	PLC_ITEM_DESC* pItemDesc = GetItemDesc(iItem);

	if (!pItemDesc)
		return;

	if (pItemDesc->dwFlags & PLC_ITEM_FLAG_ROOT)
	{
		if (IsValidItemDesc(pItemDesc))
		{
			if (pItemDesc->dwFlags & PLC_ITEM_FLAG_EXPANDED)
				CollapseItem(iItem);
			else
				ExpandItem(iItem);
		}
	}
	else
	{
		pItemDesc->dwFlags ^= PLC_ITEM_FLAG_SELECTED;
		Update(iItem);
		OnCheckItem(iItem);
	}

	SelectNext();
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if ((nChar == '+') || (nChar == '-'))
		return;

	if (nChar == VK_RETURN)
	{
		DoEdit(GetSelectionMark(), 1);
		return;
	}
	
	CListCtrl::OnChar(nChar, nRepCnt, nFlags);
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::SelectNext()
{
	int iItems	= GetItemCount();

	if (iItems > 0)
	{
		int iItem = GetSelectionMark() + 1;

		if (iItem >= iItems)
			iItem = 0;

		SetItemState(iItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
		SetSelectionMark(iItem);
		EnsureVisible(iItem, FALSE);
	}
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::DrawRootButtons(BOOL bDrawRootButtons)
{
	m_bDrawRootButtons = bDrawRootButtons;
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::DrawPropertyCells(BOOL bDrawPropertyCells)
{
	m_bDrawPropertyCells = bDrawPropertyCells;
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::DoEdit(int /*iItem*/, int /*iSubItem*/)
{
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnEndLabelEdit(pNMHDR);

	*pResult = 0;
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::OnEndLabelEdit(NMHDR* /*pNMHDR*/)
{
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::AllowItemSelection(BOOL bAllowItemSelection)
{
	m_bAllowItemSelection = bAllowItemSelection;
}
/////////////////////////////////////////////////////////////////////////////
UINT CPropertyListCtrl::OnGetDlgCode() 
{
	if (m_bEnableItemEdit)
		return DLGC_WANTALLKEYS;

	return CListCtrl::OnGetDlgCode();
}
/////////////////////////////////////////////////////////////////////////////
BOOL CPropertyListCtrl::PreTranslateMessage(MSG* pMsg) 
{
	TrackSelectionChange();
	
	return CListCtrl::PreTranslateMessage(pMsg);
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::AllowEmptyRootExpand(BOOL bAllowEmptyRootExpand)
{
	m_bAllowEmptyRootExpand = bAllowEmptyRootExpand;
}
/////////////////////////////////////////////////////////////////////////////
int CPropertyListCtrl::HitTestOnSubitem(CPoint point)
{
	LVHITTESTINFO lvhti;
	lvhti.pt = point;
	SubItemHitTest(&lvhti);

	int iSubitem = lvhti.iSubItem;

	CRect viewRect  = GetItemsRect();

	if (!viewRect.PtInRect(point))
		return -1;

	return iSubitem;
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::TrackSelectionChange()
{
	int iItem = GetFocusedItem();

	if (iItem != m_iSelectedItem)
	{
		m_iSelectedItem = iItem;

		GetParent()->PostMessage(UM_SEL_CHANGED, iItem, (LPARAM)this);

		OnSelectionChanged(iItem);
	}
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::OnSelectionChanged(int /*nSel*/)
{
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::SetResourceHandle(HINSTANCE hModule)
{
	m_imageList.SetResourceHandle(hModule);
}
/////////////////////////////////////////////////////////////////////////////
CList<PLC_ITEM_DESC*,PLC_ITEM_DESC*>* CPropertyListCtrl::GetItemsList()
{
	return &m_itemsList;
}
/////////////////////////////////////////////////////////////////////////////
DWORD CPropertyListCtrl::Blend(DWORD dwColor0, DWORD dwColor1, float fltRatio)
{
	LONG r0	= 0xFF & (dwColor0);
	LONG g0	= 0xFF & (dwColor0>>8);
	LONG b0	= 0xFF & (dwColor0>>16);

	LONG r1	= 0xFF & (dwColor1);
	LONG g1	= 0xFF & (dwColor1>>8);
	LONG b1	= 0xFF & (dwColor1>>16);

	LONG r		= min(255, (DWORD)(r0 + (r1 - r0) * fltRatio));
	LONG g		= min(255, (DWORD)(g0 + (g1 - g0) * fltRatio));
	LONG b		= min(255, (DWORD)(b0 + (b1 - b0) * fltRatio));

	return (r | (g<<8) | (b<<16));
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::EnableMultiSelection(BOOL bEnableMultiSelection)
{
	m_bEnableMultiSelection = bEnableMultiSelection;
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::InvalidateBgndRgn()
{
	CRgn bgndRgn; GetBgndRgn(bgndRgn);

	InvalidateRgn(&bgndRgn, TRUE);
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::GetBgndRgn(CRgn &bgndRgn)
{
	CRect clientRect; GetClientRect(&clientRect);

	bgndRgn.CreateRectRgn(clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);

	CRect viewRect = GetItemsRect();

	if (!viewRect.IsRectEmpty())
	{
		CRgn viewRgn; viewRgn.CreateRectRgn(viewRect.left, viewRect.top, viewRect.right, viewRect.bottom);

		bgndRgn.CombineRgn(&bgndRgn, &viewRgn, RGN_DIFF);
	}

	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	
	if (pHeaderCtrl)
	{
		CRect headerRect; pHeaderCtrl->GetWindowRect(&headerRect); ScreenToClient(&headerRect);

		CRgn headerRgn;
		headerRgn.CreateRectRgn(headerRect.left, headerRect.top, headerRect.right, headerRect.bottom);

		bgndRgn.CombineRgn(&bgndRgn, &headerRgn, RGN_DIFF);
	} 
}
/////////////////////////////////////////////////////////////////////////////
BOOL CPropertyListCtrl::IsHeaderDragging()
{
	return m_bHeaderDragging;
}
/////////////////////////////////////////////////////////////////////////////
int CPropertyListCtrl::GetFocusedItem()
{
	if (m_bEnableMultiSelection)
	{
		for (int iItem=0; iItem<GetItemCount(); iItem++)
		{
			if (GetItemState(iItem, LVIS_FOCUSED))
				return iItem;
		}
	}

	return GetSelectionMark();
}
/////////////////////////////////////////////////////////////////////////////
void CPropertyListCtrl::EnableItemEdit(BOOL bEnableItemEdit)
{
	m_bEnableItemEdit = bEnableItemEdit;
}
/////////////////////////////////////////////////////////////////////////////
int CPropertyListCtrl::GetSelCount()
{
	int iResult = 0;

	POSITION dstPos = m_itemsList.GetHeadPosition();

	while (dstPos)
	{
		PLC_ITEM_DESC* pItemDesc = m_itemsList.GetNext(dstPos);

		if (pItemDesc->dwFlags & PLC_ITEM_FLAG_SELECTED)
			iResult++;
	}

	return iResult;
}
/////////////////////////////////////////////////////////////////////////////
