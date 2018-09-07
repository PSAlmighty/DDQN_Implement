#include <stdio.h>
#include "CEnv.h"
#include "/usr/local/include/tf/tensorflow/core/platform/env.h"
#include "/usr/local/include/tf/tensorflow/core/public/session.h"
#include "/usr/local/include/tensorflow/c/c_api.h"

using namespace tensorflow;


int main() {
	vector<PriceElement> pPriceEle;
	vector<Eigen::VectorXd,Eigen::aligned_allocator<Eigen::VectorXd>> mEgMatIndicator;
	vector<Eigen::VectorXd,Eigen::aligned_allocator<Eigen::VectorXd>> mEgMatStdIndicator;
	vector<Eigen::VectorXd,Eigen::aligned_allocator<Eigen::VectorXd>> mEgIndiTensor;

	printf("Hello from TensorFlow C library version %s\n", TF_Version());

	mEgIndiTensor.clear();
	mEgMatIndicator.clear();
	mEgMatStdIndicator.clear();

	Session * sess;
	Status status = NewSession(SessionOptions(),&sess);
	if(!status.ok()){
		std::cout<<status.ToString()<<"\n";
		return 0;
	}
	else{
		printf("Sess created\n");
	}

	Tensor x(tensorflow::DT_FLOAT,tensorflow::TensorShape({1,2}));

	//int xx= CEnv_Get_Data(pPriceEle);
	for(int i = 0 ; i < 200 ; i++){
		PriceElement tmp;
		char s[5];
		sprintf(s,"%d",i);
		string is = s;
		tmp.TimeStamp = "testing" + is ;
		tmp.Open = i;
		tmp.High= i;
		tmp.Low= i;
		tmp.Close= i;
		pPriceEle.push_back(tmp);
	}

	int kk= CEnv_IndicatorMatrix(pPriceEle,mEgMatIndicator);


	for(vector<Eigen::VectorXd,Eigen::aligned_allocator<Eigen::VectorXd>>::iterator vcit = mEgMatIndicator.begin();
		vcit != mEgMatIndicator.end();
		++vcit){
		int m = 0;
		for(int i = 0; i < vcit->size() ; i++){
			if((*vcit)(i) != 0){
				m++;
			}
		}
		cerr<<m<<endl;
	}

	int a = CEnv_Standarelize(mEgMatIndicator,mEgMatStdIndicator,26);

	for(vector<Eigen::VectorXd,Eigen::aligned_allocator<Eigen::VectorXd>>::iterator vcit = mEgMatIndicator.begin();
		vcit != mEgMatIndicator.end();
		++vcit){
		int m = 0;
		for(int i = 0; i < vcit->size() ; i++){
			if((*vcit)(i) != 0){
				m++;
			}
		}
		cerr<<m<<endl;
	}

	int b = CEnv_DataIloc(mEgMatStdIndicator,mEgIndiTensor,15,60);
	tensorflow::Tensor TestTensor;
	//int c = CEnv_GenTensor(mEgMatStdIndicator,TestTensor,1,10);

	//cerr<<*p<<endl;

	return 0;

}

