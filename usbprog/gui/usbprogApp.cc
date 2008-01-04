/*
 * (c) 2007-2008, Robert Schilling
 *                Bernhard Walle <bernhard.walle@gmx.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "usbprogApp.h"
#include "usbprogFrm.h"

IMPLEMENT_APP(usbprogFrmApp)

/* -------------------------------------------------------------------------- */
bool usbprogFrmApp::OnInit()
{
    usbprogFrm* frame = new usbprogFrm(NULL);
    frame->Show(true);
    SetTopWindow(frame);
    return true;
}

/* -------------------------------------------------------------------------- */
int usbprogFrmApp::OnExit()
{
	return 0;
}

// vim: set sw=4 ts=4 fdm=marker et:
