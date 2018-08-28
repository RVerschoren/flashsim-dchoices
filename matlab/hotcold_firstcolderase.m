function [WA,M,steps]=hotcold_firstcolderase(b,d,r,f,rho_range,maxstep,stopcrit)

%b=32;
%r=0.8;
%f=0.8;
%rho_range=0.92;
%rho_range=0.80:0.01:0.94;
%d=10;

if nargin < 6
  maxstep=0.05;
end

% some useful matrices
T=toeplitz(zeros(b+1,1),0:b)*(1-r)/(1-f);
Delta=diag(0:b)*r/f;
Deltap=diag(1:b,1)*r/f;
Imin=diag(ones(1,b),-1);
TImin=T*Imin;
B=zeros(b+1,b+1);
B(b+1,b+1)=1;
for i=b:-1:1
    B(i:b+1,i)=[0; (1-r)*B(i+1:b+1,i+1)]+[r*B(i+1:b+1,i+1); 0];
end
Indx=reshape(1:(b+1)^2,b+1,b+1);
for i=1:b
    Bindx{i}=sort(reshape(spdiags(Indx(b-i+1:b+1,:),0:b-i),1,(i+1)*(b-i+1)));
    Bvals{i}=B(Bindx{i});
    Pindx{i}=sort(reshape(spdiags(Indx(1:i+1,:),0:b-i),1,(i+1)*(b-i+1)))';
end    


% iterate
% % stopcrit=10^(-7);
rho_pos=1;
for rho=rho_range
    % Initialize
    %M=rand(b+1);
    %M=M.*triu(ones(b+1,b+1));
    %M=M/sum(sum(M))
    %rho=sum(M)*(0:b)'/b
    %f=((0:b)*sum(M,2)/b)/rho
    M=zeros(b+1,b+1);
    M(1,1)=1-rho;
    M(1,b+1)=rho*(1-f);
    M(b+1,b+1)=rho*f;
%     w=uniformRW(b,d,rho);
%     M=zeros(b+1,b+1);
%     M(1,1)=w(1);
%     for i=2:b+1
%         M(1:i,i)=binopdf(0:i-1,i-1,f)'*w(i);
%     end
    
    stepsize=0.01;
    fm=ones(b+1,b+1);
    ssafm=sum(sum(abs(fm)));
    steps(rho_pos)=0;
    while (ssafm > stopcrit)
        ssafmold=ssafm;
        fmold=fm;
    
        tmp=fliplr(cumsum(fliplr(sum(M))));
        tmp=tmp.^d-[tmp(2:end) 0].^d;
        P=zeros(b+1,b+1);
        if (sum(M) > 0)
            P=M*diag(tmp./sum(M));
        else
            sumM=sum(M);
            indx=find(sumM>0);
            P(:,indx)=M(:,indx)*diag(tmp(indx)./sumM(indx));
        end
        %P=M; %RANDOM
        meanfree=sum(P)*(b:-1:0)'/(b*rho);
        
        %faster way
        fm=(Deltap*M*Imin+TImin.*(M*Imin)-Delta*M-T.*M)*meanfree-P;
        %slow obvious way
        %for j=0:b-1
        %    for i=0:j
        %        fm2(i+1,j+1)=((i+1)*r*M(i+2,j+2)/f+(j+1-i)*(1-r)*M(i+1,j+2)/(1-f)-...
        %            i*r*M(i+1,j+1)/f-(j-i)*(1-r)*M(i+1,j+1)/(1-f))*meanfree-P(i+1,j+1);
        %    end
        %end
        %j=b;
        %for i=0:j
        %    fm2(i+1,j+1)=(-i*r*M(i+1,j+1)/f-(j-i)*(1-r)*M(i+1,j+1)/(1-f))*meanfree-P(i+1,j+1);
        %end
        
        %faster way
        fm(1,b+1)=fm(1,b+1)+sum(P(1,:).*B(b+1,:));
        for i=1:b
            fm(i+1,b+1)=fm(i+1,b+1)+Bvals{i}*P(Pindx{i});
        end
                
        %slow obvious way
        %for i=0:b
        %    s=0;
        %    for k=0:i
        %        for j=i-k:b-k
        %            s=s+P(i-k+1,j+1)*B(b-k+1,j+1);
        %        end
        %    end
        %    fm(i+1,b+1)=fm(i+1,b+1)+s;
        %end
        ssafm=sum(sum(abs(fm)));
        if (ssafmold < ssafm)
            M=Mold;
            stepsize=max(0.001,stepsize/2);
            M=M+stepsize*fmold;
        else
            stepsize=min([maxstep,stepsize*1.01]);
            Mold=M;
            M=M+stepsize*fm;
        end
        %M=M+0.005*fm;
%         if (mod(steps(rho_pos),5000)==1)
%             ssafm
%         end
        steps(rho_pos)=steps(rho_pos)+1;
    end
    WA(rho_pos)=b/(b-sum(P)*(0:b)');
%     steps
%     sum(sum(P,2))
    rho_pos=rho_pos+1;
end    

WA