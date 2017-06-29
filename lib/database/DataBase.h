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
#ifndef _DataBase_h_
#define _DataBase_h_

#include <mysql.h>

#include <iostream>
#include <string.h>
#include <string>
#include <vector>
#include <stdlib.h>

//#define DEBUG_QUERIES
#ifdef DEBUG_QUERIES
#include "server/Log.h"
#endif

extern __thread MYSQL * m_connection;

namespace yucode {
		
	/// General propose DataBase execption class
	/// @author David Yuste 
	class DataBaseException : public std::exception
	{
	public:
		DataBaseException (std::string msg) : m_msg (msg) {}
		virtual ~DataBaseException () throw() {}

		virtual const char* what() const throw()
			{ return m_msg.c_str(); }

	private:
		std::string m_msg; 
	};

	/// Data base connection interface
	// @author David Yuste
	class DataBase
	{
	public:
		DataBase () {
			setAccess(NULL, NULL, NULL);
		}
		
		inline static DataBase& singleton (){
			return m_data_base;
		}

		char * getDataBaseName() {
			return database;
		}

		void setAccess(const char * d, const char * u, const char * p) { 
			if (database) free(database);
			database = d? strdup(d) : NULL; 
			if (dbuser) free(dbuser);
			dbuser = u? strdup(u) : null; 
			if (passw) free(passw);
			passw = p? strdup(p) : null;
		}

		/// MySQL connction is returned. If it doesn't exist, it is opened.
		inline MYSQL * getConnection () {
			if (!m_connection)
				openConnection();
			return m_connection;
		}
		void releaseConnection (MYSQL * connection);

	public:
		bool existsResultsForQuery(const char * query);
		inline bool existsResultsForQuery(const std::string& query) {
			return existsResultsForQuery(query.c_str());
		}
		
		inline void executeQuery(const char * query){
			if (!m_connection) openConnection();
#ifdef DEBUG_QUERIES
			LOG(std::string("executeQuery: ") << query);
#endif
			if (mysql_query(m_connection, query))
				reportError(query);
		}
		
		inline void executeQuery(const std::string& query){
			executeQuery(query.c_str());
		}
		
		void reportError(const char * query);
		inline MYSQL_RES * storeResult() {
			return mysql_store_result(m_connection);
		}
		
		void executeDelete(const char * table, const char * where);
		unsigned long long executeInsert(const char * table, const char * fields, const char * values);
		inline unsigned long long executeInsert(const std::string & table, const std::string & fields, const std::string & values) {
			return executeInsert(table.c_str(), fields.c_str(), values.c_str());
		}
		unsigned long long executeInsert(const std::string & table, const std::string &  fields, const std::vector<std::string> &  values);
		
		unsigned long long executeInsertIgnore(const char * table, const char * fields, const char * values);
		inline unsigned long long executeInsertIgnore(const std::string & table, const std::string & fields, const std::string & values) {
			return executeInsertIgnore(table.c_str(), fields.c_str(), values.c_str());
		}
		
		unsigned long long executeUpdate (const char * table, const char * assignments, const char * where);
		inline unsigned long long executeUpdate (const std::string & table, const std::string & assignments, const std::string & where) {
			return executeUpdate(table.c_str(), assignments.c_str(), where.c_str());
		}
		
		unsigned long long executeInsertOrUpdate (const char * table, const char * fields, const char * values, const char * assignments);
		inline unsigned long long executeInsertOrUpdate (const std::string & table, const std::string & fields, const std::string & values, const std::string & assignments) {
			return executeInsertOrUpdate (table.c_str(), fields.c_str(), values.c_str(), assignments.c_str());
		}
		
		inline unsigned long long getLastInsertedId(){
			return mysql_insert_id(m_connection);
		}
		inline unsigned long long getAffectedRows(){
			return mysql_affected_rows(m_connection);
		}
		
		bool createTemporaryTable(const char * table_name, const char * from);
		bool dropTemporaryTable(const char * table_name);
		
		std::string getTemporaryTableSelectSQL(const char * table_name);
		std::string getTemporaryTableName(const char * table_name);

		bool createTable(const char * table_name, const char * fields);
		bool createTableIfNotExists(const char * table_name, const char * fields);
		inline bool createTableIfNotExists(const std::string & table_name, const std::string & fields) {
			return createTableIfNotExists(table_name.c_str(), fields.c_str());
		}
		bool createDataBaseIfNotExists(const char * database_name);
		inline bool createDataBaseIfNotExists(const std::string & database_name) {
			return createDataBaseIfNotExists(database_name.c_str());
		}
		bool existsTable(const char * table_name);
		bool truncateTable(const char * table_name);
		bool dropTable(const char * table_name);
		
		inline std::string escapeString(std::string & str) {
			if (!m_connection) openConnection();
			char * toStr = new char[2*str.size() + 1];
			mysql_real_escape_string(m_connection, toStr, str.c_str(), str.size());
			std::string finalStr(toStr);
			delete [] toStr;
			return finalStr;
		}
	public:
		template <typename T>
		inline static T rowToScalar(const char * ptr, int len)
		{
			if (!ptr) return T(0);
			T t;
			std::istringstream stringStream(std::string(ptr, len));
			if ((stringStream >> t).fail())
				return T(0);
			return t;
		}
		
		inline void openConnection () {
			if (m_connection)
				return;
			openConnectionRaw();
		}
	protected:
		void openConnectionRaw ();
		void createConnection ();

	private:
		static DataBase  m_data_base;
		char * passw;
		char * dbuser;
		char * database;
	};

}

#endif
 
