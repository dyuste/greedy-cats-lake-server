/*
 *  Copyright (c) 2015 David Yuste Romero
 *
 *  THIS MATERIAL IS PROVIDED AS IS, WITH ABSOLUTELY NO WARRANTY EXPRESSED
 *  OR IMPLIED.  ANY USE IS AT YOUR OWN RISK.
 *
 *  Permission is hereby granted to use or copy this program
 *  for any purpose,  provided the above notices are retained on all copies.
 *  Permission to modify the code and to distribute modified code is granted,
 *  provided the above notices are retained, and a notice that the code was
 *  modified is included with the above copyright notice.
 */
#ifndef YUCODE_PACKAGE_HEADER_H
#define YUCODE_PACKAGE_HEADER_H

#include <map>
#include <set>
#include <vector>
#include <string>
#include <memory>

namespace yucode {
namespace jsonservice {

class ModelInterface;

typedef std::vector<std::shared_ptr<ModelInterface> > ModelObjectList;
typedef std::map<std::string, ModelObjectList> ModelObjectListCollection;

class Header {
public:
	void writeJson(std::ostream&, bool minified) const;
	
	template <typename ModelType>
	void addModelObject(const std::string &collection, const ModelType & modelObject) {
		ModelObjectListCollection::iterator modelObjectListCollectionIt = modelObjectListCollection_.find(collection);
		if (modelObjectListCollectionIt != modelObjectListCollection_.end()) {
			modelObjectListCollectionIt->second.push_back(
				std::shared_ptr<ModelInterface>(
					static_cast<ModelInterface*>(new ModelType(modelObject))));
		} else {
			ModelObjectList modelObjectList;
			modelObjectList.push_back(
				std::shared_ptr<ModelInterface>(
					static_cast<ModelInterface*>(new ModelType(modelObject))));
			modelObjectListCollection_.insert(std::make_pair<std::string, ModelObjectList>((std::string)collection, (ModelObjectList)modelObjectList));
		}
	}
	
private:
	ModelObjectListCollection modelObjectListCollection_;
};

}
}

#endif
