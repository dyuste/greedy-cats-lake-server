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
#include "TableTraverser.h"

#include <string>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include "server/Log.h"

using namespace std;

namespace yucode {

TableTraverser::~TableTraverser ()
{
	// Unfetch remaining entries, if any
	if (m_connection && m_result)
		while (m_row) m_row = mysql_fetch_row(m_result);

	if (m_result)
		mysql_free_result(m_result);

	if (m_connection)
		DataBase::singleton().releaseConnection (m_connection);
}

unsigned long long TableTraverser::getAvailableRows() 
{
	if (m_row_start != 1 || !m_row || !m_result) {
		initializeDataSource();
		advanceDataSource();
	}
	
	if (!m_row || !m_result)
		return 0;
	else
		return mysql_num_rows(m_result);
}
		

// Gets the next trunk of entries and free the previous
// ones (MYSQL_RES) if any
void TableTraverser::initializeDataSourceTrunk ()
{
	if (!m_connection)
		m_connection = DataBase::singleton().getConnection ();
	
	if (m_result) {
		while (m_row) m_row = mysql_fetch_row(m_result);
		mysql_free_result (m_result);
		m_result = 0;
	}
	
	stringstream ss;
	ss << m_sql_query << " LIMIT " << m_row_start << ", " << m_chunk_size;
	string query = ss.str();
#ifdef _DEBUG_
	cout << "[TableTraverser] Launch query: '" << query << "'" << endl;
#endif
	DataBase::singleton().executeQuery(query.c_str());
	
	m_result = DataBase::singleton().storeResult();
	if (m_result == NULL)
	{
		const char * msg = mysql_error(m_connection);
		LOG_ERROR("mysql error " << (msg ? msg : "null"));
		throw "TableTraverser::initializeDataSource() - Failed: mysql_use_result() ";
	}
}

}

