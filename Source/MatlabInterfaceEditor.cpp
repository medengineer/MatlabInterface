/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2019 Open Ephys

------------------------------------------------------------------

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <stdio.h>
#include "MatlabInterfaceEditor.h"
#include "MatlabInterface.h"

MatlabInterfaceEditor::MatlabInterfaceEditor(MatlabInterface* parentNode)
	: GenericEditor(parentNode)

{
	desiredWidth = 250;

	addTextBoxParameterEditor(Parameter::PROCESSOR_SCOPE, "host", 15, 29);
	addTextBoxParameterEditor(Parameter::PROCESSOR_SCOPE, "port", 15, 54);
	addSelectedStreamParameterEditor(Parameter::STREAM_SCOPE, "stream", 15, 79);
	addSelectedChannelsParameterEditor(Parameter::STREAM_SCOPE, "channels", 15, 104);

	for (auto& p : { "host", "port", "stream", "channels" })
    {
        auto* ed = getParameterEditor (p);
        ed->setBounds (ed->getX(), ed->getY(), desiredWidth, ed->getHeight());
    }
}