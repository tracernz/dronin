/**
******************************************************************************
*
* @file       diagnostics.h
* @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
* @author     Tau Labs, http://taulabs.org, Copyright (C) 2013
* @brief
* @see        The GNU Public License (GPL) Version 3
* @defgroup   TLMapWidget
* @{
*
*****************************************************************************/
/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H
#include <QString>
struct diagnostics
{
    diagnostics();
    int networkerrors;
    int emptytiles;
    int timeouts;
    int runningThreads;
    int tilesFromMem;
    int tilesFromNet;
    int tilesFromDB;
    QString toString()
    {
        return QString("Network errors:%1\nEmpty Tiles:%2\nTimeOuts:%3\nRunningThreads:%4\nTilesFromMem:%5\nTilesFromNet:%6\nTilesFromDB:%7").arg(networkerrors).arg(emptytiles).arg(timeouts).arg(runningThreads).arg(tilesFromMem).arg(tilesFromNet).arg(tilesFromDB);
       ;
    }
};

#endif // DIAGNOSTICS_H

/**
 * @}
 * @}
 */