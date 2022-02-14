
#include "sql_work.h"


//global session, in future on raspberry pi could just set up a database that doesn't require password
Session my_session(/*SERVER*/, 33060, /*USER*/, /*PASSWORD*/, /*DATABASE*/);

//song_query takes the name of a song and a mysql session object in order to query songs that are "LIKE" (in sql context) the string passed in and returns a vector of Row objects
std::vector<Row> song_query(string& song, Session& sql_session) {
    //Append different strings to make full query for MySQL song_list table in karaoke database
    string query = "SELECT CONVERT(@row_number:=@row_number+1, SIGNED) as row_num, song_name, song_artist from song_list, (SELECT @row_number:=0) as t WHERE song_name LIKE \"%" ;
    string end = "%\" LIMIT 10;";
    query = query.append(song + end);
    //Create an object that has the results of the query
    auto query_result = sql_session.sql(query).execute();
    //Return a vector with all entries of the resulting query, individual rows can be accessed with .get(x) member or [] operator 
    return query_result.fetchAll();
}

