// Endpoints are either sources or destinations for data
// They include a file endpoint, a PE-File endpoint, a dummy endpoint, and a combination endpoint

#pragma once

// The endpoint interface
public interface class Endpoint {
public:
	// Returns true if this endpoint can handle the specification given
	virtual bool IsSpec(System::String ^spec) = 0;
	// Adds the item 
	virtual void Add(System::String ^spec, void *data, size_t size, int overwrite) = 0;
	// Gets the item
	virtual void *Get(System::String ^spec, size_t *size) = 0;
	// Removes the item
	virtual void Remove(System::String ^spec) = 0;
	
	//virtual void Clean(System::String ^spec) = 0;
	
	// Commits all edited files
	virtual void Commit() = 0;
};

// An endpoint that accepts all types and just throws errors; used as a fallback
ref class DummyEndpoint : public Endpoint {
public:
	virtual bool IsSpec(System::String ^spec);
	virtual void Add(System::String ^spec, void *data, size_t size, int overwrite);
	virtual void *Get(System::String ^spec, size_t *size);
	virtual void Remove(System::String ^spec);
	//virtual void Clean(System::String ^spec);
	virtual void Commit();
};

// An endpoint for modifying the filesystem
ref class Files : public Endpoint {
public:
	virtual bool IsSpec(System::String ^spec); // accepts legit filenames
	virtual void Add(System::String ^spec, void *data, size_t size, int overwrite); // saves file
	virtual void *Get(System::String ^spec, size_t *size); // read file
	virtual void Remove(System::String ^spec); // deletes file
	//virtual void Clean(System::String ^spec);
	virtual void Commit(); // does nothing, everything is committed immediately
};

// An endpoint for modifying resources in PE files (exe, dll, sys, ocx, ...)
ref class PEFiles : public Endpoint {
	System::Collections::Generic::Dictionary<System::String^,System::IntPtr> ^peFiles;
	System::Collections::Generic::Dictionary<System::String^,bool> ^edited;

	static array<System::String^> ^pe_exts = {
		L".exe", L".dll", L".sys", L".ocx", L".mui", L".drv", L".cpl", L".efi", L".com", L".fnt", L".msstyles", L".scr", L".ax", L".acm", L".ime", L".pe"
	};

	void *GetPEFile(System::String ^filename);

public:
	PEFiles();
	virtual bool IsSpec(System::String ^spec); // example: file.exe|BITMAP|100|1033
	virtual void Add(System::String ^spec, void *data, size_t size, int overwrite); // add a new resource
	virtual void *Get(System::String ^spec, size_t *size); // read a resource
	virtual void Remove(System::String ^spec); // removes a resource
	//virtual void Clean(System::String ^spec);
	virtual void Commit(); // saves all the files with modified resources
};

// An endpoint for modifying resources in RES files
ref class RESFiles : public Endpoint {
	System::Collections::Generic::Dictionary<System::String^,System::IntPtr> ^resFiles;
	System::Collections::Generic::Dictionary<System::String^,bool> ^edited;

	void *GetRESFile(System::String ^filename);

public:
	RESFiles();
	virtual bool IsSpec(System::String ^spec); // example: file.res|BITMAP|100|1033
	virtual void Add(System::String ^spec, void *data, size_t size, int overwrite); // add a new resource
	virtual void *Get(System::String ^spec, size_t *size); // read a resource
	virtual void Remove(System::String ^spec); // removes a resource
	//virtual void Clean(System::String ^spec);
	virtual void Commit(); // saves all the files with modified resources
};

// This endpoint is actually a collection of endpoints, and looks for the one to use based on IsSpec()
ref class Endpoints : public Endpoint {
	array<Endpoint^> ^endpoints;
public:
	Endpoints();
	Endpoint ^GetEndpoint(System::String ^spec);
	virtual bool IsSpec(System::String ^spec);
	virtual void Add(System::String ^spec, void *data, size_t size, int overwrite);
	virtual void *Get(System::String ^spec, size_t *size);
	virtual void Remove(System::String ^spec);
	//virtual void Clean(System::String ^spec);
	virtual void Commit();
};
