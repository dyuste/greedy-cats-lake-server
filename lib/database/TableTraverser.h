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
#ifndef _TableTraverser_h_
#define _TableTraverser_h_

#include "DataBase.h"

namespace yucode {
	
	class TableTraverser {
	public:
		TableTraverser (const std::string& sql_query)
			: m_sql_query(sql_query), 
			  m_connection(0), m_result(0), 
			  m_lengths(0), m_row(0), m_row_start(0),
			  m_chunk_size(2000), m_limit(0), m_total_rows(0) {}
		
		virtual ~TableTraverser ();

		void setLimit(unsigned long long limit) { m_limit = limit; }
		void setChunkSize(unsigned long long chunkSize) { m_chunk_size = chunkSize; }
		unsigned long long getAvailableRows();
		
		// STL-iterator class, single instance allowed
		class iterator : public std::iterator<std::input_iterator_tag, const std::string> {
		public:
			iterator (TableTraverser & container, bool end = false)
				: m_container(container), m_end(end) {}
			iterator (const iterator & it) : m_container(it.m_container), m_end(it.m_end) {}
			inline iterator & operator= (const iterator & it) {this->m_container = it.m_container; this->m_end = it.m_end; return *this; }
			inline iterator & operator++ () {
				m_container.advanceDataSource();
				m_end = m_container.eofDataSource();
				return *this;
			}
			inline iterator operator++ (int) { iterator tmp(*this); operator++(); return tmp; }
			inline bool operator== (const iterator& rhs) { return rhs.m_end == m_end; }
			inline bool operator!= (const iterator& rhs){ return rhs.m_end != m_end; }

			inline bool isNull(int row) const { return m_container.m_row[row] == 0; }
			
			inline unsigned int rowLength(int row) const 
				{ return m_container.m_lengths[row]; }
				
			inline const char * rowAsString(int row) const
				{ return m_container.m_row[row]? m_container.m_row[row] : ""; }
				
			inline int rowAsInt(int row) const
				{ return DataBase::rowToScalar<int>(m_container.m_row[row],
				m_container.m_lengths[row]); }
				
			inline unsigned int rowAsUnsignedInt(int row) const
				{ return DataBase::rowToScalar<unsigned int>(m_container.m_row[row],
				m_container.m_lengths[row]); }
				
			inline long long rowAsLong(int row) const
				{ return DataBase::rowToScalar<long>(m_container.m_row[row],
				m_container.m_lengths[row]); }	
			
			inline long long rowAsUnsignedLong(int row) const
				{ return DataBase::rowToScalar<unsigned long>(m_container.m_row[row],
				m_container.m_lengths[row]); }	
			
			inline long long rowAsLongLong(int row) const
				{ return DataBase::rowToScalar<long long>(m_container.m_row[row],
				m_container.m_lengths[row]); }	
			
			inline long long rowAsUnsignedLongLong(int row) const
				{ return DataBase::rowToScalar<unsigned long long>(m_container.m_row[row],
				m_container.m_lengths[row]); }	
				
			inline float rowAsFloat(int row) const
				{ return DataBase::rowToScalar<float>(m_container.m_row[row],
				m_container.m_lengths[row]); }
		
			inline float rowAsDouble(int row) const
				{ return DataBase::rowToScalar<double>(m_container.m_row[row],
				m_container.m_lengths[row]); }
				
			TableTraverser & m_container;
			bool m_end;
		};
		
		// Main class iteration

		/// Data source iteration begins
		/// Data source is initialized (TableTraverser::initializeDataSource()), and 
		/// advanced up to the first register (TableTraverser::advanceDataSource).
		/// The returned iterator' end state is set by calling TableTraverser::eofDataSourcE() 
		inline iterator begin ()
		{
			if (m_row_start != 1 || !m_row) {
				initializeDataSource();
				advanceDataSource();
			}
			return iterator((TableTraverser&)*this, eofDataSource());
		}

		inline iterator end () {
			return iterator((TableTraverser&)*this, true);
		}

	protected:
		// Initialise data source (eg., connect DB and launch an SQL query)
		void initializeDataSource (){
			if (m_result) {
				while (m_row) m_row = mysql_fetch_row(m_result);
				mysql_free_result (m_result);
				m_result = 0;
			}
			m_row = 0;
			m_row_start = 0;
			m_total_rows = 0;
		}

		// Fetch the next data element
		// If previous register is not set, it starts by the first.
		// Previous register, if any, must be released.
		inline void advanceDataSource () {
			if (m_row_start % m_chunk_size == 0)
				initializeDataSourceTrunk ();
			m_row_start++;
			m_total_rows++;

			m_row =	mysql_fetch_row(m_result);
			m_lengths = mysql_fetch_lengths(m_result);
		}

		// Returns whether the last element has been read
		inline bool eofDataSource () { return m_row == 0 || (m_limit && m_total_rows > m_limit); }

	private:
		// Gets the next trunk of entries and free the previous
		// ones (MYSQL_RES) if any
		void initializeDataSourceTrunk();

	protected:
		std::string m_sql_query;
		MYSQL * m_connection;
		MYSQL_RES * m_result;
		unsigned long * m_lengths;
                MYSQL_ROW m_row;
		unsigned long long m_row_start;
		unsigned long long m_chunk_size;
		unsigned long long m_limit;
		unsigned long long m_total_rows;
	};
	
}

#endif

