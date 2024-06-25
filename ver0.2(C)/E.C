/*         "E.COM" Ver 0.02      */
/*   Execute from Archive file�@ */
/*       by autumn   Nov.1989    */


#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<process.h>
#include<alloc.h>


#define DEBUG


/*�g�p����A�[�J�C�o�[�̖��O*/
#define ARCHIVER "LHARC"

/*�@�Q�Ƃ�����ϐ����@*/
#define ENV_EAPATH	"EAPATH"	/*�A�[�J�C�u�t�@�C�������p�X*/
#define ENV_EATMP	"EATMP"		/*�R�}���h�W�J�P���f�B���N�g��*/

/*�@���ϐ�EAPATH����`����Ă��Ȃ��Ƃ��̃f�t�H���g�����p�X�@*/
#define DEFAULT_EAPATH	".\\STDCMD"	/* �J�����g��"STDCMD.LZH" */



typedef int EXIT_CODE;


char eatmp[130], eapath[130];


/* �f�o�b�O�p�̂����������������@*/
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



/*�@�Z�~�R�����ŋ�؂�ꂽ������̍��ڐ��𐔂���@*/
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


/*�@�Z�~�R�����ŋ�؂�ꂽ����������ڂ��Ƌ�؂�@*/
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
	/*�@���ϐ��d�`�s�l�o��{���A���o���@*/
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

	/*�@���ϐ��d�`�o�`�s�g�𓾂�@*/
	ecp = getenv( ENV_EAPATH );
	if( ecp != NULL ) {
		strcpy( eapath, ecp );
	} else {
		strcpy( eapath, DEFAULT_EAPATH );
	}
}


/*�@LHARC���N�����Ďw��R�}���h���𓀂���@*/
/* lharc e /m/n/e%extract_path %arcfile %cmd_name.* */

void extract_cmd( char * cmd_name )
{
	static char * lharc_par[] = {
		ARCHIVER,
		"e",	/* extract command */
		NULL,	/*����́A����������̈�̗\��i�X�C�b�`�j*/
		NULL,	/*����́A����������̈�̗\��i���Ƀt�@�C���j*/
		NULL,	/*����́A����������̈�̗\��i�𓀃t�@�C���j*/
		NULL	/*�����������̃^�[�~�l�[�^�[*/
	};
	char p_buf[130], c_buf[130];
	int envc;
	char **envv;
	int i, result;

	/*�@�k�g�������p�����[�^�t�B�[���h���쐬����@*/
	if( strlen( eatmp ) != 0 ) {
		sprintf(p_buf, "/m/n/e%s", eatmp );
		lharc_par[2] = p_buf;
	} else {
		lharc_par[2] = "/m/n";
	}

	/*�@�d�`�o�`�s�g��؂蕪����@*/
	envc = count_env( eapath );
	envv = cut_env( eapath );

	/*�@�ǂ݂����R�}���h���𓾂�@*/
	strcpy( c_buf, cmd_name );
	strcat( c_buf, ".*" );
	lharc_par[4] = c_buf;

	/*�@���ԂɃA�[�J�C�u�t�@�C������ǂ݂����Ă݂�@*/
	for(i=0; i<envc; i++){
		lharc_par[3] = envv[i];
#ifdef DEBUG
		display_args( ARCHIVER, lharc_par );
#endif
		result = spawnvp( P_WAIT, ARCHIVER, lharc_par );
		if( result == -1 ) {
			perror( ARCHIVER "���Ăяo���܂���" );
			exit( 2 );
		}
		if( result == 0 ) return;	/*��������==���܂��𓀂ł���*/
	}
	printf("�R�}���h[%s]��ǂݏo�����Ƃ��ł��܂���B\n",cmd_name);
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

	if( exit_code == -1 ) {		/* spawn�ŋN���ł��Ȃ��Ƃ��͂������Ɖ���@*/
		/*�@COMMAND.COM�o�R�ł������R�}���h���N������@*/
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
			perror("COMMAND.COM��������܂���");
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
		printf("�@�\�F�A�[�J�C�u�t�@�C���̒��̃R�}���h���N�����܂��B\n");
		exit(2);
	}

	get_environ();

	extract_cmd( argv[1] );

	exit_code = execute_cmd( argv[1], argc - 1, &argv[1] );

	delete_cmd( argv[1] );

	return( exit_code );
}
