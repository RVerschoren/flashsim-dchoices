%% Select FTL
b=32;
ftlhcwf='hcwf';fieldshcwf = {'fair';'end'};withhcswap=true;
ftldwf='dwf';fieldsdwf = {'fair';'end'};withhcswap=false;
% ftl=ftlhcwf;fields=fieldshcwf;
ftl=ftldwf;fields=fieldsdwf;
% ftl='cold';
rho=0.90;
stepsize=10^-2;
stopcrit=10^-7;

savetofile = true;

% Wear leveling schemes
ban='ban';
maxval='maxval';
swap='swap';
hcswap='hcswp';
resultsdir='../results/';

reffile = strcat(resultsdir,ftl,'trace/',ftl,'trace-b32-d10-sf0.10-f0.010');
refleg = 'd = 10, NoWL';
%% Plot Movie trace
traceid ='Movi';
for d=[10 100]
    p=[0.5 0.1 0.05 0.01 0.005 0.001];
    plotWLvl(resultsdir, reffile, refleg, ftldwf, d, ban, traceid, fieldsdwf, p, savetofile)
    plotWLvl(resultsdir, reffile, refleg, ftlhcwf, d, ban, traceid, fieldshcwf, p, savetofile)
    % plotWLvl(resultsdir, reffile, refleg, ftl, d, swap, traceid, fields, p, savetofile)
    plotWLvl(resultsdir, reffile, refleg, ftlhcwf, d, hcswap, traceid, fieldshcwf, p, savetofile)
end

%% Plot Booting trace
traceid ='Boot';

for d=[10 100]
    p=[0.5 0.1 0.05 0.01 0.005 0.001];
    plotWLvl(resultsdir, reffile, refleg, ftldwf, d, ban, traceid, fieldsdwf, p, savetofile)
    plotWLvl(resultsdir, reffile, refleg, ftlhcwf, d, ban, traceid, fieldshcwf, p, savetofile)
    % plotWLvl(resultsdir, reffile, refleg, ftl, d, swap, traceid, fields, p, savetofile)
    plotWLvl(resultsdir, reffile, refleg, ftlhcwf, d, hcswap, traceid, fieldshcwf, p, savetofile)
end

% %% Plot Inst trace
% Missing reference
% for d=[10 100]
%     traceid ='Inst';
% 
%     p=[0.5 0.1 0.05 0.01 0.005 0.001];
%     plotWLvl(resultsdir, reffile, refleg, ftldwf, d, ban, traceid, fieldsdwf, p, savetofile)
%     plotWLvl(resultsdir, reffile, refleg, ftlhcwf, d, ban, traceid, fieldshcwf, p, savetofile)
%     plotWLvl(resultsdir, reffile, refleg, ftlhcwf, d, hcswap, traceid, fieldshcwf, p, savetofile)
% end

% %% Plot prxy0 trace (done)
% traceid ='prxy';
% 
% p=[0.5 0.1 0.05 0.01 0.005 0.001];
% plotWLvl(resultsdir, reffile, refleg, ftl, d, ban, traceid, fields, p, savetofile)
% % plotWLvl(resultsdir, reffile, refleg, ftl, d, swap, traceid, fields, p, savetofile)
% if(withhcswap)
%     plotWLvl(resultsdir, reffile, refleg, ftl, d, hcswap, traceid, fields, p, savetofile)
% end

%% Plot synth data
% for d=[10 100]
%     %p=[0.5 0.1 0.05 0.01 0.005 0.001];
%     p=[0.1 0.01 0.001];
%     f=[0.01 0.10];%0.05 0.10 0.20];
%     r=1-f;
%     for it=1:numel(f)
%         rstr=num2str(r(it), '%4.3f');
%         fstr=num2str(f(it), '%4.3f');
%         reffile = strcat(resultsdir, ftl,'/',ftl,'-b32-d10-sf0.10-f',fstr, '-r',rstr);
% %         plotWLvlSynth(resultsdir, reffile, refleg, ftldwf, d, ban, f(it), r(it), fieldsdwf, p , savetofile);
% %         plotWLvlSynth(resultsdir, reffile, refleg, ftlhcwf, d, ban, f(it), r(it), fieldshcwf, p , savetofile);
% %         plotWLvlSynth(resultsdir, reffile, refleg, ftlhcwf, d, hcswap, f(it), r(it), fieldshcwf, p , savetofile);
%         for pb=p
%             [WAban, confban]=computeWA(resultsdir, reffile, ftlhcwf, d, ban, f(it), r(it), fieldshcwf, pb);
%             WAode = hotcold_HCWF_ban(b, d, r(it), f(it), rho, pb, stepsize, stopcrit);
%             {'BAN' d rho r(it) f(it) pb  WAode WAban confban}
%         end
%         for ps=p
%             [WAswp, confswp]=computeWA(resultsdir, reffile, ftlhcwf, d, hcswap, f(it), r(it), fieldshcwf, ps);
%             WAodeswp = hotcold_HCWF_hcswap(b, d, r(it), f(it), rho, ps, stepsize, stopcrit);
%             {'HCSWAP' d rho r f ps  WAodeswp WAswp confswp}
%         end
%     end
% end

%% DWF HCWF d=10
% % for p in 0.5 0.1 0.05 0.01 0.005 0.001;
% % ./SimBanProb.exe DWF 32 10 0.1 0.01 0.99 1 10 10000 5000 $p;
% % ./SimBanProb.exe HCWF 32 10 0.1 0.01 0.99 1 10 10000 5000 $p;
% % ./SimHCSwap.exe  HCWF 32 10 0.1 0.01 0.99 1 10 10000 5000 $p;
% % end
% d=10;
% p=[0.5 0.1 0.05 0.01 0.005 0.001];
% f=[0.01 0.05 0.10 0.20];
% r=1-f;
% for it=1:numel(f)
%     rstr=num2str(r(it), '%4.3f');
%     fstr=num2str(f(it), '%4.3f');
%     reffile = strcat(resultsdir, ftl,'/',ftl,'-b32-d10-sf0.10-f',fstr, '-r',rstr);
%     plotWLvlSynth(resultsdir, reffile, refleg, ftl, d, ban, f(it), r(it), fields, p , savetofile);
%     if(withhcswap)
%         plotWLvlSynth(resultsdir, reffile, refleg, ftl, d, hcswap, f(it), r(it), fields, p , savetofile);
%     end
% end


%% DWF f=0.01 Ban anomalie
% ftl='dwf';
% d=100;
% p=[0.9 0.7 0.5 0.3 0.1];
% %p=[0.5 0.1 0.05 0.01 0.005 0.001];
% f=0.01;
% r=1-f;
% rstr=num2str(r, '%4.3f');
% fstr=num2str(f, '%4.3f');
% reffile = strcat(resultsdir, ftl,'/',ftl,'-b32-d10-sf0.10-f',fstr, '-r',rstr);
% plotWLvlSynth(resultsdir, reffile, refleg, ftl, d, ban, f, r, fields, p , savetofile);



%% Helper Functions
% function plotTau(resultsdir,reffileformat, refleg, ftl, traceid, fields, t, savetofile)
%     tau = cellfun(@(x) num2str(x), num2cell(t'), 'UniformOutput', false);
%     if(max(t)-min(t) > 1000)
%         leg = cat(1, refleg,  strcat({'tau = '}, num2str(t','%4.0e')));
%     else
%         leg = cat(1, refleg,  strcat({'tau = '}, num2str(t','%d')));
%     end
%     
%     for fieldit = 1:size(fields,1)
%         field = fields{fieldit};
%         switch field
%             case 'fair'
%                 fieldAxis = 'Fairness';
%             case 'end'
%                 fieldAxis = 'Endurance';
%             otherwise
%                 fieldAxis = 'First Cold Block Erase';
%         end
%         reffile = strcat(reffileformat, '-',traceid, '-', field, '.*.csv');
%         
%         if(savetofile)
%             savelocation = strcat(resultsdir,ftl,'tracebantau-b32-sf0.10-f0.010-',traceid, '-', field);
%         else
%             savelocation='';
%         end
%         
%         tauc = strcat(resultsdir,ftl,'traceban/',ftl,'traceban-b32-d100-sf0.10-f0.010-tau', tau, '-',traceid,'-',field,'.*.csv');
%         files = cat(1, reffile, tauc);
%         if(isequal(field,'colderase'))
%             barUnmergedFiles(savelocation, files, {'MaxPE',fieldAxis, strcat(ftl, ' ', wlvl,' ', traceid, ' d=100')}, leg, '', [14 14]);
%         else
%             plotUnmergedFiles(savelocation, files, {'MaxPE',fieldAxis, strcat(ftl, ' ', wlvl,' ', traceid, ' d=100')}, leg, '', {}, [14 14]);
%         end
%     end
% end


function plotWLvl(resultsdir,reffileformat, refleg, ftl, d, wlvl, traceid, fields, pin, savetofile)
    p = cellfun(@(x) num2str(x,'%4.3f'), num2cell(pin'), 'UniformOutput', false);
    leg = cat(1, refleg,  strcat({'p = '}, num2str(pin','%4.3f')));
    
    for fieldit = 1:size(fields,1)
        field = fields{fieldit};
        switch field
            case 'fair'
                fieldAxis = 'Fairness';
            case 'end'
                fieldAxis = 'Endurance';
            otherwise
                fieldAxis = 'First Cold Block Erase';
        end
        
        reffile = strcat(reffileformat, '-',traceid, '-', field, '.*.csv');
        
        if(savetofile)
            savelocation = strcat(resultsdir,ftl,'trace',wlvl,'-b32-d',num2str(d),'-sf0.10-f0.010-',traceid, '-', field);
        else
            savelocation='';
        end
        
        pc = strcat(resultsdir,ftl,'trace',wlvl,'/',ftl,'trace',wlvl,'-b32-d',num2str(d),'-sf0.10-f0.010-p', p, '-',traceid,'-',field,'.*.csv');
        files = cat(1, reffile, pc);
        if(isequal(field,'colderase'))
            barUnmergedFiles(savelocation, files, {'MaxPE',fieldAxis, strcat(ftl, ' ', wlvl,' ', traceid, ' d=',num2str(d))}, leg, '', [14 14]);
        else
            plotUnmergedFiles(savelocation, files, {'MaxPE',fieldAxis, strcat(ftl, ' ', wlvl,' ', traceid, ' d=',num2str(d))}, leg, '', {}, [14 14]);
        end

%         pc = strcat(ftl,'traceban/',ftl,'traceban-b32-greedy-sf0.10-f0.010-p', p, '-',traceid,'-',field,'.*.csv');
%         files = cat(1, reffile, pc);
%         plotUnmergedFiles(savelocation, files, {'MaxPE',fieldAxis, strcat(traceid, ' Greedy')}, leg, '', {}, [14 14]);
    end
end

function plotWLvlSynth(resultsdir,reffileformat, refleg, ftl, d, wlvl, fin, rin, fields, pin, savetofile)
    p = cellfun(@(x) num2str(x,'%4.3f'), num2cell(pin'), 'UniformOutput', false);
    leg = cat(1, refleg,  strcat({'p = '}, num2str(pin','%4.3f')));
    f = num2str(fin,'%4.3f');
    r = num2str(rin,'%4.3f');
    
    for fieldit = 1:size(fields,1)
        field = fields{fieldit};
        switch field
            case 'fair'
                fieldAxis = 'Fairness';
            case 'end'
                fieldAxis = 'Endurance';
            otherwise
                fieldAxis = 'First Cold Block Erase';
        end
        reffile = strcat(reffileformat, '-', field, '.*.csv');
        
        if(savetofile)
            savelocation = strcat(resultsdir,ftl,wlvl,'-b32-d',num2str(d),'-sf0.10-f',f,'-r',r, '-', field);
        else
            savelocation='';
        end
        
        pc = strcat(resultsdir,ftl,wlvl,'/',ftl,wlvl,'-b32-d',num2str(d),'-sf0.10-f', f, '-r', r, '-p', p,'-',field,'.*.csv');
        files = cat(1, reffile, pc);
        if(isequal(field,'colderase'))
            barUnmergedFiles(savelocation, files, {'MaxPE',fieldAxis, strcat(ftl, ' ', wlvl,' f= ',f, ', r= ', r, ', d=',num2str(d))}, leg, '', [14 14]);
        else
            plotUnmergedFiles(savelocation, files, {'MaxPE',fieldAxis, strcat(ftl,' ', wlvl,' f= ',f, ', r= ', r, ', d=',num2str(d))}, leg, '', {}, [14 14]);
        end
    end
end

function [WA, conf]=computeWA(resultsdir,reffileformat, ftl, d, wlvl, fin, rin, fields, pin)
    p = cellfun(@(x) num2str(x,'%4.3f'), num2cell(pin'), 'UniformOutput', false);
    f = num2str(fin,'%4.3f');
    r = num2str(rin,'%4.3f');
    field='WA'; 
    reffile = strcat(reffileformat, '-', field, '.*.csv');
        
        
    pc = strcat(resultsdir,ftl,wlvl,'/',ftl,wlvl,'-b32-d',num2str(d),'-sf0.10-f', f, '-r', r, '-p', p,'-',field,'.*.csv');
%     files = cat(1, reffile, pc);
    [WA,conf]=mean_and_conf_int(pc);
end
