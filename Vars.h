#pragma once

// Save variables, and process strings with %# (e.g. %1) replacing variables

ref class Vars {
	int count;
	array<System::Collections::Generic::List<System::String^>^> ^vars;
	// Prepares strings with % symbols, converts all %# (where # is 0 to 9) to non-printing characters, and %% to %
	// Note: a particular %# will not be converted if there are no variables set for that index
	System::Collections::Generic::List<int> ^PrepString(System::String ^%s);
public:
	Vars();
	~Vars();

	// Reads an XML node "<Vars index="#">" and loads the values for a particular variable index
	void Set(System::Xml::XmlElement ^node);
	
	// Expand the variables in a src/dest pair, returning src / list of dest pairs
	// Notes:
	//   1) If a variable exists in the src and all dests, then it is expanded simultaneously in all, creating new pairs of src / dest
	//   2) If a variable is in src and not in all dests, it is only allowed to have a single value 
	System::Collections::Generic::Dictionary<System::String^, System::Collections::Generic::List<System::String^>^>^
		Vars::Expand(System::String^ src, System::Collections::Generic::List<System::String^> ^dests);

	// Expand the variables in a single string, returning a list of expanded strings
	System::Collections::Generic::List<System::String^> ^Vars::Expand(System::String^ name);
};
