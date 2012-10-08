/*
 * AppMgrRegistry.h
 *
 *  Created on: Oct 4, 2012
 *      Author: vsalo
 */

#ifndef APPMGRREGISTRY_H_
#define APPMGRREGISTRY_H_

#include "IApplication.h"
#include "RegistryItem.h"

#include <map>
#include <string>

class AppMgrRegistry
{
public:
	~AppMgrRegistry( );

	static AppMgrRegistry& getInstance( );

	const RegistryItem& registerApplication( const IApplication& app );
	void unregisterApplication( RegistryItem& item  );

	const RegistryItem& getItem( const IApplication& app ) const;
	const RegistryItem& getItem( const std::string& app ) const;

private:
	AppMgrRegistry( );

	std::map<std::string, RegistryItem> mRegistryItems;
};

#endif /* APPMGRREGISTRY_H_ */
