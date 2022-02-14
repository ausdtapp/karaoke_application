#pragma once


#include <iostream>
#include <mysqlx/xdevapi.h>

using namespace ::mysqlx;

extern Session my_session;

std::vector<Row> song_query(string& song, Session& sql_session);