#include "Vars.h"

using namespace System;
using namespace System::Xml;
using namespace System::Collections::Generic;

Vars::Vars() {
	vars = gcnew array<List<String^>^>(10);
	for (int i = 0; i < 10; i++)
		vars[i] = nullptr;
	count = 0;
}
Vars::~Vars() {
	for (int i = 0; i < 10; i++)
		delete vars[i];
	delete[] vars;
}

// Sets up variables from an XML node
void Vars::Set(XmlElement ^node) {
	int i = Convert::ToInt32(node->GetAttribute(L"index"));
	if (vars[i] == nullptr) {
		vars[i] = gcnew List<String^>(node->ChildNodes->Count);
		count++;
	} else {
		vars[i]->Clear();
	}
	for each (XmlElement ^n in node->SelectNodes(L"Name")) {
		vars[i]->Add(n->InnerText);
	}
}

// Swap two references
generic<typename T> void Swap(T % a, T % b){
	T c = a;
	a = b;
	b = c;
}

List<int> ^Vars::PrepString(String ^%s) {
	s = s->Replace(L"%%", L"\13");
	List<int> ^indicies = gcnew List<int>(count);
	for (int i = 0; i < 10; i++) {
		if (vars[i] == nullptr)
			continue;
		String ^x = L"%"+i;
		if (s->Contains(x)) {
			String ^y = gcnew String((wchar_t)(i==0?10:i), 1);
			s = s->Replace(x, y);
			indicies->Add(i);
		}
	}
	s = s->Replace(L"\13", L"%");
	return indicies;
}

// Takes the intersection between two ordered sets (return a n b (where n is the intersection symbol))
List<int> ^Intersection(List<int> ^a, List<int> ^b) {
	List<int> ^x = gcnew List<int>(MIN(a->Count, b->Count));
	int ai = 0, bi = 0;
	while (ai < a->Count && bi < b->Count) {
		if (a[ai] == b[bi]) {
			x->Add(a[ai++]);
			bi++;
		} else if (a[ai] < b[bi]) {
			ai++;
		} else /*if (a[ai] > b[bi])*/ {
			bi++;
		}
	}
	return x;
}

// Takes the difference between two ordered sets (return a - b)
List<int> ^Difference(List<int> ^a, List<int> ^b) {
	List<int> ^x = gcnew List<int>(a->Count - b->Count);
	int ai = 0, bi = 0;
	while (ai < a->Count && bi < b->Count) {
		if (a[ai] == b[bi]) {
			ai++;
			bi++;
		} else if (a[ai] < b[bi]) {
			x->Add(a[ai++]);
		} else /*if (a[ai] > b[bi])*/ {
			bi++;
		}
	}
	while (ai < a->Count) {
		x->Add(a[ai++]);
	}
	return x;
}

Dictionary<String^, List<String^>^> ^Vars::Expand(String^ src, List<String^> ^dests) {
	List<int> ^src_i = PrepString(src);
	List<List<int>^> ^dest_i = gcnew List<List<int>^>(dests->Count);

	List<int> ^common = gcnew List<int>(src_i);

	for (int i = 0; i < dests->Count; i++) {
		String ^s = dests[i];
		dest_i->Add(PrepString(s));
		common = Intersection(common, dest_i[i]);
		dests[i] = s;
	}

	// do all src 'internal' substitions
	src_i = Difference(src_i, common);
	for each (int i in src_i) {
		if (vars[i]->Count > 1)
			return nullptr; // error! cannot have multiple sources to a single destination
		String ^y = gcnew String((wchar_t)(i==0?10:i), 1);
		src = src->Replace(y, vars[i][0]);
	}

	// do all dests 'internal' substitions
	List<String^> ^tmp = gcnew List<String^>();
	List<String^> ^list = gcnew List<String^>();
	List<String^> ^final = gcnew List<String^>();
	for (int i = 0; i < dests->Count; i++) {
		dest_i[i] = Difference(dest_i[i], common);
		list->Add(dests[i]);
		for each (int j in dest_i[i]) {
			String ^y = gcnew String((wchar_t)(j==0?10:j), 1);
			for each (String ^s in vars[j]) {
				for each (String ^t in list) {
					tmp->Add(t->Replace(y, s));
				}
			}
			Swap<List<String^>^>(list, tmp);
			tmp->Clear();
		}
		final->AddRange(list);
		list->Clear();
	}
	Swap<List<String^>^>(dests, final);
	delete tmp;
	delete list;
	delete final;

	// do joint (common) substitions
	Dictionary<String^, List<String^>^> ^dict = gcnew Dictionary<String^, List<String^>^>();
	Dictionary<String^, List<String^>^> ^temp = gcnew Dictionary<String^, List<String^>^>();
	dict[src] = dests;

	for each (int i in common) {
		String ^y = gcnew String((wchar_t)(i==0?10:i), 1);
		for each (KeyValuePair<String^,List<String^>^> ^kv in dict) {
			String ^x = kv->Key;
			List<String^> ^l = kv->Value;
			for each (String ^s in vars[i]) {
				src = x->Replace(y, s);
				if (!temp->ContainsKey(src))
					temp[src] = gcnew List<String^>(l->Count);
				for each (String ^t in l) {
					temp[src]->Add(t->Replace(y, s));
				}
			}
		}
		Swap<Dictionary<String^, List<String^>^>^>(dict, temp);
		temp->Clear();
	}

	delete temp;
	return dict;
}

List<String^> ^Vars::Expand(String^ name) {
	List<String^> ^list = gcnew List<String^>();
	List<String^> ^temp = gcnew List<String^>();
	List<int> ^indicies = PrepString(name);
	list->Add(name);
	for each (int i in indicies) {
		String ^y = gcnew String((wchar_t)(i==0?10:i), 1);
		for each (String ^s in vars[i]) {
			for each (String ^t in list) {
				temp->Add(t->Replace(y, s));
			}
		}
		Swap<List<String^>^>(list, temp);
		temp->Clear();
	}
	delete temp;
	return list;
}
