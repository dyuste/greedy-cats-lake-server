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
#include "DataBase.h"
#include "server/Log.h"

#include <sstream>
#include <iostream>
#include <stdio.h>
#include <cstring>
#include <errmsg.h>
#include "misc/Utilities.h"

using namespace std;
using namespace yucode;

extern __thread long long yucode::server::LogSeqNumber;

/// @class DataBase
/// Provides centralized management of data base connections. Client objects
/// must use this class to get and release data base connections.
/// Since DataBase::DataBase() is private, it can't instantiated by client
/// objects. A single instance is allowed, and can be acceded through
/// DataBase::singleton().


/// Singleton object
DataBase  DataBase::m_data_base;

__thread MYSQL * m_connection = 0;

/// Nothing is done in current implementation. A single connection is
/// hold during the whole program execution
void DataBase::releaseConnection (MYSQL * connection)
{
	// Doing nothing, reuse connection (release at destroy)
}

/// MySQL connection is opened.
void DataBase::openConnectionRaw ()
{
	createConnection();

	if (mysql_real_connect(m_connection, "localhost", dbuser,
				passw, database, 0, NULL, 0) == NULL)
	{
		const char * msg = mysql_error(m_connection);
		LOG_ERROR("[DataBase] mysql error: " << string(msg ? msg : "null"));

		mysql_close(m_connection);
		m_connection = NULL;
		
		// Try to create data base
		createConnection();
		if (mysql_real_connect(m_connection, "localhost", dbuser,
				passw, NULL, 0, NULL, 0) == NULL) {
			const char * msg = mysql_error(m_connection);
			LOG_ERROR("[DataBase] mysql error (failed to connect to any database): "
				<< string(msg ? msg : "null"));
			
			throw "DataBase::openConnection() - Failed mysql_real_connect()";
		}
		createDataBaseIfNotExists(database);
		mysql_close(m_connection);
		m_connection = NULL;
		
		// Last attempt
		createConnection();
		if (mysql_real_connect(m_connection, "localhost", dbuser,
				passw, database, 0, NULL, 0) == NULL)
		{
			const char * msg = mysql_error(m_connection);
			LOG_ERROR("[DataBase] mysql error (Second attempt): "
				<< string(msg ? msg : "null"));
			
			throw "DataBase::openConnection() - Failed mysql_real_connect()";
		}
	}

	LOG("[DataBase] Connected: " << dbuser << "@" << database);
}

void DataBase::createConnection ()
{
	m_connection = mysql_init(NULL);
	if (!m_connection)
		throw "DataBase::openConnection() - Failed mysql_init()";

	mysql_options(m_connection, MYSQL_SET_CHARSET_NAME, "utf8");
	mysql_options(m_connection, MYSQL_INIT_COMMAND, "SET NAMES utf8");
	mysql_set_character_set(m_connection, "utf8_unicode_ci");
}

void DataBase::reportError(const char * query) {
	// Recover in case of lost connection
	unsigned int err = mysql_errno(m_connection);
	if (err == CR_SERVER_GONE_ERROR || err == CR_SERVER_LOST) {
		m_connection = NULL;
		openConnection();
		if (!mysql_query(m_connection, query))
			return;
	}
	LOG_ERROR("DataBase SQL error message: " << mysql_error(m_connection));
	LOG_ERROR("DataBase SQL error query: " << query);
	throw "DataBase query failed";
}

void DataBase::executeDelete (const char * table, const char * where)
{
	char buffer[10240];
	
	if (!m_connection) openConnection();

	snprintf(buffer, sizeof(buffer), "DELETE FROM %s WHERE %s ", table, where);

	executeQuery(buffer);
}

bool DataBase::existsResultsForQuery(const char * query) {
	executeQuery(query);
	
	MYSQL_RES * result = mysql_use_result(m_connection);
	if (result == NULL) {
		const char * msg = mysql_error(m_connection);
		LOG_ERROR("mysql error " << (msg ? msg : "null"));
		throw "TableTraverser::existsResultsForQuery() - Failed: mysql_use_result() ";
	}
	MYSQL_ROW row = mysql_fetch_row(result);
	bool exists = row? true : false;
	mysql_free_result(result);
	
	return exists;
}

unsigned long long DataBase::executeInsert (const char * table, const char * fields, const char * values)
{
	char buffer[10240];
	
	if (!m_connection) openConnection();

	snprintf(buffer, sizeof(buffer), "INSERT INTO %s (%s) VALUES %s ", table, fields, values);

	executeQuery(buffer);

	return mysql_insert_id(m_connection);
}

unsigned long long DataBase::executeInsertIgnore (const char * table, const char * fields, const char * values)
{
	char buffer[10240];
	
	if (!m_connection) openConnection();

	snprintf(buffer, sizeof(buffer), "INSERT IGNORE INTO %s (%s) VALUES %s ", table, fields, values);

	executeQuery(buffer);

	return mysql_insert_id(m_connection);
}

unsigned long long DataBase::executeInsert(const std::string & table, const std::string &  fields, const std::vector<std::string> &  values) {
	std::stringstream ss;
	std::string separator;
	ss << "(";
	for (std::vector<std::string>::const_iterator it = values.begin(); it != values.end(); ++it) {
		// Escape characters
		std::string value = *it;
		misc::Utilities::stringReplace(value, "\\", "\\\\");
		misc::Utilities::stringReplace(value, "'", "\\'");
		ss << separator << "'" << value << "'";
		separator = ",";
	}
	ss << ")";
	
	return executeInsert(table.c_str(), fields.c_str(), ss.str().c_str());
}
		

unsigned long long DataBase::executeUpdate (const char * table, const char * assignments, const char * where)
{
	char buffer[10240];
	if (!m_connection) openConnection();
	
	snprintf(buffer, sizeof(buffer), "UPDATE %s SET %s WHERE %s ", table, assignments, where);

	executeQuery(buffer);
	
	return mysql_affected_rows(m_connection);
}

unsigned long long DataBase::executeInsertOrUpdate (const char * table, const char * fields, const char * values, const char * assignments)
{
	char buffer[10240];
	if (!m_connection) openConnection();
		
	snprintf(buffer, sizeof(buffer), "INSERT INTO %s (%s) VALUES %s ON DUPLICATE KEY UPDATE %s ", table, fields, values, assignments);

	executeQuery(buffer);
	
	return mysql_affected_rows(m_connection);
}

bool DataBase::createTable(const char * table_name, const char * fields)
{
	char buffer[10240];
	
	if (!m_connection) openConnection();

	snprintf(buffer, sizeof(buffer), "DROP TABLE IF EXISTS %s ", table_name);
	if (mysql_query(m_connection, buffer))
	{
		cerr << "[ERROR] DataBase::createTable() at mysql_query(): " << mysql_error(m_connection) << endl
		     << "Query was:" << buffer << endl;
		return false;
	}

	snprintf(buffer, sizeof(buffer), "CREATE TABLE %s (%s) "
	                "ENGINE = MYISAM CHARACTER  SET utf8 COLLATE utf8_unicode_ci AUTO_INCREMENT=0", table_name, fields);
	if (mysql_query(m_connection, buffer))
	{
		cerr << "[ERROR] DataBase::createTable() at mysql_query(): " << mysql_error(m_connection) << endl
		     << "Query was:" << buffer << endl;
		return false;
	}
	return true;
}

bool DataBase::createTableIfNotExists(const char * table_name, const char * fields)
{
	char buffer[10240];
	
	if (!m_connection) openConnection();

	snprintf(buffer, sizeof(buffer), "CREATE TABLE IF NOT EXISTS %s (%s) "
	                "ENGINE = MYISAM CHARACTER SET utf8 COLLATE utf8_unicode_ci AUTO_INCREMENT=0", table_name, fields);
	if (mysql_query(m_connection, buffer))
	{
		cerr << "[ERROR] DataBase::createTableIfNotExists() at mysql_query(): " << mysql_error(m_connection) << endl
		     << "Query was:" << buffer << endl;
		return false;
	}
	return true;
}

bool DataBase::createDataBaseIfNotExists(const char * database_name)
{
	char buffer[10240];
	
	if (!m_connection) openConnection();

	snprintf(buffer, sizeof(buffer), "CREATE DATABASE IF NOT EXISTS %s "
	                "CHARACTER SET utf8 COLLATE utf8_unicode_ci", database_name);
	if (mysql_query(m_connection, buffer))
	{
		cerr << "[ERROR] DataBase::createDataBaseIfNotExists() at mysql_query(): " << mysql_error(m_connection) << endl
		     << "Query was:" << buffer << endl;
		return false;
	}
	return true;
}

bool DataBase::existsTable(const char * table_name) {
	char buffer[10240];
	
	if (!m_connection) openConnection();

	snprintf(buffer, sizeof(buffer), "SELECT 1 "
		"FROM INFORMATION_SCHEMA.TABLES "
		"WHERE TABLE_TYPE='BASE TABLE' " 
		"AND TABLE_NAME='%s'", table_name);
	
	if (mysql_query(m_connection, buffer))
	{
		cerr << "[ERROR] DataBase::existsTable() at mysql_query(): " << mysql_error(m_connection) << endl
		     << "Query was:" << buffer << endl;
		return false;
	}
	
	
	MYSQL_RES * result = storeResult();
	if (!result) {
		cerr << "[ERROR] DataBase::existsTable() at store result: " << mysql_error(m_connection) << endl
		     << "Query was:" << buffer << endl;
		return false;
	}
	
	MYSQL_ROW row = mysql_fetch_row(result);
	if (row && row[0] && atoi(row[0]) == 1)
		return true;
	
	return false;
}

bool DataBase::truncateTable(const char * table_name)
{
	char buffer[10240];
	
	if (!m_connection) openConnection();

	snprintf(buffer, sizeof(buffer), "TRUNCATE TABLE %s", table_name);
	if (mysql_query(m_connection, buffer))
	{
		cerr << "[ERROR] DataBase::truncateTable() at mysql_query(): " << mysql_error(m_connection) << endl
		     << "Query was:" << buffer << endl;
		return false;
	}
	return true;
}

bool DataBase::dropTable(const char * table_name)
{
	char buffer[10240];
	
	if (!m_connection) openConnection();

	snprintf(buffer, sizeof(buffer), "DROP TABLE IF EXISTS %s ", table_name);
	if (mysql_query(m_connection, buffer))
	{
		cerr << "[ERROR] DataBase::dropTable() at mysql_query(): " << mysql_error(m_connection) << endl
		     << "Query was:" << buffer << endl;
		return false;
	}
	return true;
}

bool DataBase::createTemporaryTable(const char * table_name, const char * from)
{
	char buffer[10240];
	
	if (!m_connection) openConnection();

	snprintf(buffer, sizeof(buffer), "DROP TABLE IF EXISTS %s_%lld ", table_name, yucode::server::LogSeqNumber);
	
	if (mysql_query(m_connection, buffer))
	{
		cerr << "[ERROR] DataBase::createTemporaryTable() at mysql_query(): " << mysql_error(m_connection) << endl
		     << "Query was:" << buffer << endl;
		return false;
	}

	snprintf(buffer, sizeof(buffer), "CREATE TEMPORARY TABLE %s_%lld ENGINE = MYISAM CHARACTER  SET utf8 COLLATE utf8_unicode_ci AUTO_INCREMENT=0 AS (%s) ",
		table_name, yucode::server::LogSeqNumber, from);
	if (mysql_query(m_connection, buffer))
	{
		cerr << "[ERROR] DataBase::createTemporaryTable() at mysql_query(): " << mysql_error(m_connection) << endl
		     << "Query was:" << buffer << endl;
		return false;
	}
	return true;
}

std::string DataBase::getTemporaryTableSelectSQL(const char * table_name) {
	std::stringstream sql;
	sql << "SELECT * FROM " << table_name << "_" << yucode::server::LogSeqNumber;
	return sql.str();
}

std::string DataBase::getTemporaryTableName(const char * table_name) {
	std::stringstream sql;
	sql << table_name << "_" << yucode::server::LogSeqNumber;
	return sql.str();
}

bool DataBase::dropTemporaryTable(const char * table_name)
{
	char buffer[10240];
	
	if (!m_connection) openConnection();

	snprintf(buffer, sizeof(buffer), "DROP TABLE IF EXISTS %s_%lld ", table_name, yucode::server::LogSeqNumber);
	if (mysql_query(m_connection, buffer))
	{
		cerr << "[ERROR] DataBase::dropTemporaryTable() at mysql_query(): " << mysql_error(m_connection) << endl
		     << "Query was:" << buffer << endl;
		return false;
	}
	return true;
}
