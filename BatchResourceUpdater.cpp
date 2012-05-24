// BatchResourceUpdater: program for automated reading, writing, and removing resources from pe-files
// Copyright (C) 2012  Jeffrey Bush  jeff@coderforlife.com
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// This is the source code file for the entry point into the program
// This reads the BRU/XML file, utilizing vars and endpoints to do most of the actual work

#include "resource.h"
#include "general.h"
#include "Vars.h"
#include "Endpoints.h"

using namespace System;
using namespace System::IO;
using namespace System::Xml;
using namespace System::Resources;
using namespace System::Collections::Generic;

// Handles a Copy XML node
void copy(Vars ^vars, Endpoint ^endpoint, XmlElement ^node);

// Handles a Remove XML node
void remove(Vars ^vars, Endpoint ^endpoint, XmlElement ^node);

int main(array<System::String ^> ^args) {
	Console::WriteLine(L"BatchResourceUpdater Copyright (C) 2012  Jeffrey Bush <jeff@coderforlife.com>");
	Console::WriteLine(L"This program comes with ABSOLUTELY NO WARRANTY;");
	Console::WriteLine(L"This is free software, and you are welcome to redistribute it");
	Console::WriteLine(L"under certain conditions;");
	Console::WriteLine(L"See http://www.gnu.org/licenses/gpl.html for more details.");
	Console::WriteLine();

	// Check argument
	if (args->Length < 1) {
		Console::Error->WriteLine(L"You must supply an BRU/XML file on the command line.");
		return 1;
	}
	String ^file = args[0];
	if (!File::Exists(file)) {
		Console::Error->WriteLine(L"! Error: The BRU/XML file specified cannot be found.");
		return 1;
	}

	// Setup the vars and endpoints that will do almost all the work
	Vars ^vars = gcnew Vars();
	Endpoints ^endpoints = gcnew Endpoints();

	// Load the XML Schema for the BRU files
	HRSRC	schemaRsrc = FindResource(NULL, MAKEINTRESOURCE(IDR_SCHEMA), L"SCHEMA");
	char*	schemaData = (char*)LockResource(LoadResource(NULL, schemaRsrc));
	DWORD	schemaSize = SizeofResource(NULL, schemaRsrc);

	try {
		// Prepare the XML Reader
		XmlReaderSettings ^settings = gcnew XmlReaderSettings();
		settings->IgnoreWhitespace = true;
		settings->ValidationType = ValidationType::Schema;
		// Parse the Schema
		settings->Schemas->Add(nullptr, gcnew XmlTextReader(gcnew StringReader(gcnew String(schemaData, 0, schemaSize))));
		// Load the BRU/XML file and validate it
		XmlReader ^reader = XmlReader::Create(file, settings);
		// Create an XmlDocument (with DOM interface)
		XmlDocument ^xml = gcnew XmlDocument();
		xml->Load(reader);
		xml->PreserveWhitespace = false;
		XmlElement ^root = xml->DocumentElement;
		// Go through the children and delegate these to extracting and updating
		for each (XmlElement ^node in root->SelectNodes(L"Var|Copy|Clear|Remove")) {
			if (node->Name == L"Var") {
				vars->Set(node);
			} else if (node->Name == L"Copy") {
				copy(vars, endpoints, node);
			} else if (node->Name == L"Clear") {
				//clear(vars, endpoints, node);
				Console::Error->WriteLine(L"* Warning: Clear currently not supported");
			} else if (node->Name == L"Remove") {
				remove(vars, endpoints, node);
			}
		}
		// Commit all the changes
		endpoints->Commit();
	} catch (Schema::XmlSchemaException ^xmlSchEx) {
		Console::Error->WriteLine(L"! Error: Validating XML File: "+xmlSchEx->Message);
	} catch (XmlException ^xmlEx) {
		Console::Error->WriteLine(L"! Error: Reading XML File: "+xmlEx->Message);
	}

	return 0;
}

// Handles a Copy XML node
void copy(Vars ^vars, Endpoint ^endpoint, XmlElement ^node) {
	// Get Source and Destinations
	String ^src = node->SelectSingleNode(L"Source")->InnerText;
	XmlNodeList ^destinations = node->SelectNodes(L"Destination");
	List<String^> ^dests = gcnew List<String^>(destinations->Count);
	for each (XmlNode ^dest in destinations) {
		dests->Add(dest->InnerText);
	}

	// Expand source and destination names
	Dictionary<String^, List<String^>^> ^files = vars->Expand(src, dests);

	// Get the overwrite setting
	String ^ow = node->GetAttribute(L"overwrite");
	PE::Overwrite overwrite = PE::ALWAYS;
	if (ow != nullptr)
		overwrite = (ow == L"never" ? PE::NEVER : (ow == L"only" ? PE::ONLY : PE::ALWAYS));

	for each (KeyValuePair<String^, List<String^>^> ^ kv in files) {
		// Get the source data
		String ^src = kv->Key;
		size_t size;
		void *data;
		try {
			data = endpoint->Get(src, &size);
		} catch (Exception ^ex) { Console::Error->WriteLine(L"! Error: Reading '"+src+L"': "+ex->Message); continue; }
		for each (String ^dest in kv->Value) {
			// Write to the destination
			Console::WriteLine(L"Copying " + src + L" to " + dest + L"...");
			try {
				endpoint->Add(dest, data, size, overwrite);
			} catch (Exception ^ex) { Console::Error->WriteLine(L"! Error: Writing '"+dest+L"': "+ex->Message); }
		}
		free(data);
	}
}

// Handles a Remove XML node
void remove(Vars ^vars, Endpoint ^endpoint, XmlElement ^node) {
	for each (String ^file in vars->Expand(node->InnerText)) {
		Console::WriteLine(L"Removing " + file + L"...");
		try {
			endpoint->Remove(file);
		} catch (Exception ^ex) { Console::Error->WriteLine(L"! Error: Removing '"+file+L"': "+ex->Message); }
	}
}
