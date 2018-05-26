// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

#include "stdafx.h"
#include <UIRibbon.h>
#include <UIRibbonPropertyHelpers.h>

#define RECENT_FILE_COUNT 10
#define MAXFILENAME 250
extern char mru[10][MAXFILENAME];

// PURPOSE: Given MFC's Recent Files list, produce the Recent Items list in the Application Menu.
// RETURNS: S_OK when the operation is successful, E_FAIL otherwise.
// NOTE:    The result is stored in the PROPVARIANT pointed to by pvarValue.
HRESULT PopulateRibbonRecentItems(PROPVARIANT* pvarValue);
