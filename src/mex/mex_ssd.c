#include "mex.h"
#include <stdio.h>

int run_SSD(const char* const, const char* const, const int, double*, double**, int*, double**, int*,
			double**, int*, double**, int*, char**, int*);

void copy_values_to_2D_mxArray(int m,double d[], mxArray** a){
	*a = mxCreateDoubleMatrix((mwSize)m, (mwSize)2, mxREAL);
	double* outarr = mxGetPr(*a);
	//for(int j = 0; j < n; j++){
	for(int i = 0; i < m; i++){
	  // Index, first column
	  outarr[i]= i;
	  // Value, second column
	  outarr[1*m+i]= (double) d[i];
	}
	//}

}

void check_input(int nlhs, mxArray *plhs[],
				 int nrhs, const mxArray *prhs[]){
	if (nrhs < 1) {
		mexErrMsgIdAndTxt( "MATLAB:mxmalloc:invalidNumInputs",
				"One input argument required.");
	}
	if (nlhs > 5) {
		mexErrMsgIdAndTxt( "MATLAB:MXMALLOC:maxlhs",
				"Too many output arguments.");
	}

	if (!mxIsChar(prhs[0]) || (mxGetM(prhs[0]) != 1 ) )  {
		mexErrMsgIdAndTxt( "MATLAB:mxmalloc:invalidInput",
				"Input argument must be a string.");
	}
}

/* The gateway function */
void mexFunction(int nlhs, mxArray *plhs[],
				 int nrhs, const mxArray *prhs[])
{
	check_input(nlhs,plhs,nrhs,prhs);

	size_t ssdarg_len = mxGetN(prhs[0]) + 1;
	char* ssdarg = mxMalloc(ssdarg_len);
	int status = mxGetString(prhs[0],ssdarg, ssdarg_len);
	if (status != 0) {
		mexErrMsgIdAndTxt( "MATLAB:mxmalloc:mxGetString",
						   "Failed to copy input string argument 0 into allocated memory.");
	}

	size_t conffile_len;
	char* conffile;

	if(nrhs < 2){
		conffile = "/home/rverschoren/Projects/MATLAB/ssd.conf";
	}else{
		conffile_len = mxGetN(prhs[1]) + 1;
		conffile = mxMalloc(conffile_len);
		status = mxGetString(prhs[1],conffile, conffile_len);
		if (status != 0) {
			mexErrMsgIdAndTxt( "MATLAB:mxmalloc:mxGetString",
							   "Failed to copy input string argument 1 into allocated memory.");
		}
	}

	//mexPrintf("The input string is:  %s\n", buf);
	double WA;
	double *end, *fair, *hotbl, *coldbl;
	int numend,numfair,numhot,numcold;

	char* error;
	int error_len;
	status = run_SSD(conffile, ssdarg, ssdarg_len, &WA,
					 &end, &numend,
					 &fair, &numfair,
					 &hotbl, &numhot,
					 &coldbl, &numcold,
					 &error, &error_len);
	if(status != 0){
		char * mexErrMsg = (char*)malloc((16+error_len) * sizeof(char));
		sprintf(mexErrMsg , "SSD run failed with status %i: %s", status, error);
		mexErrMsgIdAndTxt( "MATLAB:mxmalloc:invalidInput",
				mexErrMsg);
		return;
	}

	if(nlhs > 0){
		plhs[0] = mxCreateDoubleScalar(WA);
	}
	if(nlhs > 1){
		copy_values_to_2D_mxArray(numend, end, &(plhs[1]));
		if(status == 0 && end != NULL){
			free(end);
			end=NULL;
		}
	}
	if(nlhs > 2){
		copy_values_to_2D_mxArray(numfair, fair, &(plhs[2]));
		if(status == 0 && fair != NULL){
			free(fair);
			fair=NULL;
		}
	}
	if(nlhs > 3){
		copy_values_to_2D_mxArray(numhot, hotbl, &(plhs[3]));
		if(status == 0 && hotbl != NULL){
			free(hotbl);
			hotbl=NULL;
		}
	}
	if(nlhs > 4){
		copy_values_to_2D_mxArray(numcold, coldbl, &(plhs[4]));
		if(status == 0 && coldbl != NULL){
			free(coldbl);
			coldbl=NULL;
		}
	}

	if(nrhs >= 2) mxFree(conffile);
	mxFree(ssdarg);


/*
	if(nlhs > 0 && nrhs > 0){
		int nrows = mxGetM(prhs[0]);
		int ncols = mxGetN(prhs[0]);
		if(mxIsChar_800(prhs[0])){
			//a=mex_ssd('labla')
			//char argv[ncols] ;
			//printf(nrows); printf(ncols);
			mxGetChars
			//int status = mxGetString_800(prhs[0],argv,ncols);
			//for(int i=0; i < ncols; i++)
			//    printf(argv);

			int numberwords = run_SSD(argv, ncols);
			plhs[0] = mxCreateDoubleScalar_800(numberwords);
		}else{
			plhs[0] = mxCreateString_800("Neeneegeenchar");
			run_SSD("HCWF 32", 7);
		}
	}else{
		run_SSD("blabla",6);
	}
	*/
}

