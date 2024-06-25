/*         "E.COM" Ver 0.02      */
/*   Execute from Archive file　 */
/*       by autumn   Nov.1989    */


#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<process.h>
#include<alloc.h>


#define DEBUG


/*使用するアーカイバーの名前*/
#define ARCHIVER "LHARC"

/*　参照する環境変数名　*/
#define ENV_EAPATH	"EAPATH"	/*アーカイブファイル検索パス*/
#define ENV_EATMP	"EATMP"		/*コマンド展開１時ディレクトリ*/

/*　環境変数EAPATHが定義されていないときのデフォルト検索パス　*/
#define DEFAULT_EAPATH	".\\STDCMD"	/* カレントの"STDCMD.LZH" */



typedef int EXIT_CODE;


char eatmp[130], eapath[130];


/* デバッグ用のｆｕｎｃｔｉｏｎ　*/
#ifdef DEBUG
void display_args( char * cmd, char ** args )
{
	printf("executing : [%s]",cmd);
	while( *args != NULL ){
		printf(" %s",*args );
		args++;
	}
	printf("\n");
}
#endif



/*　セミコロンで区切られた文字列の項目数を数える　*/
int count_env( char * text )
{
	char * cp;
	int num;

	num = 1;
	cp = text;
	while(1){
		cp = strchr( cp, ';' );
		if( cp == NULL ) break;
		num++;
		cp++;
	}

	return( num );
}


/*　セミコロンで区切られた文字列を項目ごと区切る　*/
char ** cut_env( char * text )
{
	static char * envv[64];
	char ** ep;
	char * cp, * cp1, * cp2;
	cp = text;
	ep = envv;
	while(1){
		cp1 = strchr( cp, ';' );
		if( cp1 == NULL ){
			cp2 = strchr( cp, '\0' );
		} else {
			cp2 = cp1;
		}
		*ep = calloc( cp2-cp + 1, sizeof( char ) );
		if( *ep == NULL ) {
			perror("e.com");
			exit(2);
		}
		strncpy( *ep, cp, cp2-cp );
		if( cp1 == NULL ) break;
		ep++;
		cp = cp2;
	}
	return( envv );
}




void get_environ( void )
{
	char * ecp;
	/*　環境変数ＥＡＴＭＰを捜し、取り出す　*/
	ecp = getenv( ENV_EATMP );
	if( ecp != NULL ) {
		strcpy( eatmp, ecp );
		ecp = strchr( eatmp, '\0' ) - 1;
		/*if( (*ecp != '\\')&&(*ecp != ':') ) {*/
		if( *ecp != '\\' ) {
			*(ecp+1) = '\\';
			*(ecp+2) = '\0';
		}
	} else {
		strcpy( eatmp, "" );
	}

	/*　環境変数ＥＡＰＡＴＨを得る　*/
	ecp = getenv( ENV_EAPATH );
	if( ecp != NULL ) {
		strcpy( eapath, ecp );
	} else {
		strcpy( eapath, DEFAULT_EAPATH );
	}
}


/*　LHARCを起動して指定コマンドを解凍する　*/
/* lharc e /m/n/e%extract_path %arcfile %cmd_name.* */

void extract_cmd( char * cmd_name )
{
	static char * lharc_par[] = {
		ARCHIVER,
		"e",	/* extract command */
		NULL,	/*これは、書き換える領域の予約（スイッチ）*/
		NULL,	/*これは、書き換える領域の予約（書庫ファイル）*/
		NULL,	/*これは、書き換える領域の予約（解凍ファイル）*/
		NULL	/*ｓｐａｗｎのターミネーター*/
	};
	char p_buf[130], c_buf[130];
	int envc;
	char **envv;
	int i, result;

	/*　ＬＨａｒｃパラメータフィールドを作成する　*/
	if( strlen( eatmp ) != 0 ) {
		sprintf(p_buf, "/m/n/e%s", eatmp );
		lharc_par[2] = p_buf;
	} else {
		lharc_par[2] = "/m/n";
	}

	/*　ＥＡＰＡＴＨを切り分ける　*/
	envc = count_env( eapath );
	envv = cut_env( eapath );

	/*　読みだしコマンド名を得る　*/
	strcpy( c_buf, cmd_name );
	strcat( c_buf, ".*" );
	lharc_par[4] = c_buf;

	/*　順番にアーカイブファイルから読みだしてみる　*/
	for(i=0; i<envc; i++){
		lharc_par[3] = envv[i];
#ifdef DEBUG
		display_args( ARCHIVER, lharc_par );
#endif
		result = spawnvp( P_WAIT, ARCHIVER, lharc_par );
		if( result == -1 ) {
			perror( ARCHIVER "が呼び出せません" );
			exit( 2 );
		}
		if( result == 0 ) return;	/*成功した==うまく解凍できた*/
	}
	printf("コマンド[%s]を読み出すことができません。\n",cmd_name);
	exit( 2 );
}



EXIT_CODE execute_cmd( char * cmd_name, int argc, char *argv[] )
{
	char * spawn_argv[64];
	char c_buf[130];
	int exit_code;
	int i;

	strcpy( c_buf, eatmp );
	strcat( c_buf, cmd_name );

	i = 0;
	while( i<argc ){
		spawn_argv[i] = argv[i];
		i++;
	}
	spawn_argv[i] = NULL;
	spawn_argv[0] = c_buf;

#ifdef DEBUG
	display_args( c_buf, spawn_argv );
#endif
	exit_code = spawnv( P_WAIT, c_buf, spawn_argv);

	if( exit_code == -1 ) {		/* spawnで起動できないときはｂａｔと仮定　*/
		/*　COMMAND.COM経由でｂａｔコマンドを起動する　*/
		spawn_argv[0] = "COMMAND";
		spawn_argv[1] = "/c";
		spawn_argv[2] = c_buf;
		i = 1;
		while( i<argc ){
			spawn_argv[i+2] = argv[i];
			i++;
		}
		spawn_argv[i+2] = NULL;
#ifdef DEBUG
		display_args( "COMMAND", spawn_argv );
#endif
		exit_code = spawnvp( P_WAIT, "COMMAND" , spawn_argv);
		if( exit_code == -1 ) {
			perror("COMMAND.COMが見つかりません");
			exit( 2 );
		}
	}

	return( exit_code );
}



void delete_cmd( char * cmd_name )
{
	char c_buf[130];

	strcpy( c_buf, "del " );
	strcat( c_buf, eatmp );
	strcat( c_buf, cmd_name );
	strcat( c_buf, ".*" );

	system( c_buf );
}



int main( int argc, char * argv[] )
{
	EXIT_CODE exit_code;

	if( argc == 1 ){
		printf("usage : e COMMAND ARGS\n");
		printf("E.COM Ver 0.02 by autumn 1989\n");
		printf("機能：アーカイブファイルの中のコマンドを起動します。\n");
		exit(2);
	}

	get_environ();

	extract_cmd( argv[1] );

	exit_code = execute_cmd( argv[1], argc - 1, &argv[1] );

	delete_cmd( argv[1] );

	return( exit_code );
}
