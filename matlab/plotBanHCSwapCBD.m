%% Select FTL
%b=32;
b=4;
ftlhcwf='hcwf';fieldshcwf = {'fair';'end';'WA'};withhcswap=true;
ftlcold='cold';fieldscold = {'fair';'end';'WA'};withhcswap=true;
ftldwf='dwf';fieldsdwf = {'fair';'end';'WA'};withhcswap=false;

%ftlhcwf='hcwf';fieldshcwf = {'WA'};withhcswap=true;
%ftlcold='cold';fieldscold = {'WA'};withhcswap=true;
%ftldwf='dwf';fieldsdwf = {'WA'};withhcswap=false;

% ftl=ftlhcwf;fields=fieldshcwf;
%ftl=ftldwf;fields=fieldsdwf;

% ftl='cold';
rho=0.90;
savetofile = true;
stepsize=0.01;

% Wear leveling schemes
ban='ban';
hcswap='hcswp';
resultsdir='../resultscold/';

% reffile = strcat(resultsdir,ftl,'trace/',ftl,'trace-b32-d10-sf0.10-f0.010');
% refleg = 'd = 10, NoWL';
% %% Plot Movie trace
% traceid ='Movi';
% for d=[10 100]
%     p=[0.5 0.1 0.05 0.01 0.005 0.001];
%     plotWLvl(resultsdir, reffile, refleg, ftldwf, d, ban, traceid, fieldsdwf, p, savetofile)
%     plotWLvl(resultsdir, reffile, refleg, ftlhcwf, d, ban, traceid, fieldshcwf, p, savetofile)
%     % plotWLvl(resultsdir, reffile, refleg, ftl, d, swap, traceid, fields, p, savetofile)
%     plotWLvl(resultsdir, reffile, refleg, ftlhcwf, d, hcswap, traceid, fieldshcwf, p, savetofile)
% end

% %% Plot Booting trace
% traceid ='Boot';
% 
% for d=[10 100]
%     p=[0.5 0.1 0.05 0.01 0.005 0.001];
%     plotWLvl(resultsdir, reffile, refleg, ftldwf, d, ban, traceid, fieldsdwf, p, savetofile)
%     plotWLvl(resultsdir, reffile, refleg, ftlhcwf, d, ban, traceid, fieldshcwf, p, savetofile)
%     % plotWLvl(resultsdir, reffile, refleg, ftl, d, swap, traceid, fields, p, savetofile)
%     plotWLvl(resultsdir, reffile, refleg, ftlhcwf, d, hcswap, traceid, fieldshcwf, p, savetofile)
% end

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
bstr=num2str(b);
gcas={'d'}%, 'cbd'};
for gc_index=1:numel(gcas)
    gca=gcas{gc_index};
    for d=[1 3 10 30]
        dstr=num2str(d);
        gcastr=strcat(gca, dstr);
        %p=[0.5 0.1 0.05 0.01 0.005 0.001];
        %p=[0.05 0.01 0.005 0.001];
        p=[0.05 0.01];
        f=[0.01 0.10 0.20];
        r=1-f;


        for it=1:numel(f)
            rstr=num2str(r(it), '%4.3f');
            fstr=num2str(f(it), '%4.3f');
            refdwffile = strcat(resultsdir, ftldwf,'/',ftldwf,'-b',bstr,'-',gca,dstr,'-sf0.10-f',fstr, '-r',rstr);
            refhcwffile = strcat(resultsdir, ftlhcwf,'/',ftlhcwf,'-b',bstr,'-',gca,dstr,'-sf0.10-f',fstr, '-r',rstr);
            refcoldfile = strcat(resultsdir, ftlcold,'/',ftlcold,'-b',bstr,'-',gca,dstr,'-sf0.10-f',fstr, '-r',rstr);
            refleg = strcat(gca, ' = ', num2str(d), ', NoWL');
                %%plotWLvlSynth(resultsdir, refdwffile, refleg, ftldwf, gca, dstr, ban, f(it), r(it), fieldsdwf, p , savetofile);
                %%plotWLvlSynth(resultsdir, refhcwffile, refleg, ftlhcwf, gca, dstr, ban, f(it), r(it), fieldshcwf, p , savetofile);
                %%plotWLvlSynth(resultsdir, refhcwffile, refleg, ftlhcwf, gca, dstr, hcswap, f(it), r(it), fieldshcwf, p , savetofile);
                %%plotWLvlSynth(resultsdir, refcoldfile, refleg, ftlcold, gca, dstr, ban, f(it), r(it), fieldshcwf, p , savetofile);
                %%plotWLvlSynth(resultsdir, refcoldfile, refleg, ftlcold, gca, dstr, hcswap, f(it), r(it), fieldshcwf, p , savetofile);
            %plot2WLvlSynth(resultsdir, refdwffile, strcat(ftldwf,' NoWL'), refhcwffile, strcat(ftlhcwf,' NoWL'), ftldwf, ftlhcwf, gca, dstr, ban, ban, f(it), r(it), fieldshcwf, p , savetofile);
            %plot2WLvlSynth(resultsdir, refhcwffile, strcat(ftlhcwf,' NoWL'), refcoldfile, strcat(ftlcold,' NoWL'), ftlhcwf, ftlcold, gca, dstr, ban, ban, f(it), r(it), fieldshcwf, p , savetofile);
            %plot2WLvlSynth(resultsdir, refhcwffile, strcat(ftlhcwf,' NoWL'), refcoldfile, strcat(ftlcold,' NoWL'), ftlhcwf, ftlcold, gca, dstr, hcswap, hcswap, f(it), r(it), fieldshcwf, p , savetofile);

        
%             [WAban, confban]=computeWANoWLvl(resultsdir, ftlhcwf, gca, d, f(it), r(it));
%             WAode = hotcold_separ_HWFCWF_fast(b, d, r(it), f(it), rho, stepsize);
%             {'NoWlvl' d rho r(it) f(it)  WAode WAban confban}
%             for pb=p
%                 [WAban, confban]=computeWA(resultsdir, ftlhcwf, gca, d, ban, f(it), r(it), pb);
%                 WAode = hotcold_HCWF_ban(b, d, r(it), f(it), rho, pb, stepsize);
%                 {'BAN' d rho r(it) f(it) pb  WAode WAban confban}
%             end
%             for ps=p
%                 [WAswp, confswp]=computeWA(resultsdir, ftlhcwf, gca, d, hcswap, f(it), r(it), ps);
%                 WAodeswp = hotcold_HCWF_hcswap(b, d, r(it), f(it), rho, ps, stepsize);
%                 {'HCSWAP' d rho r(it) f(it) ps  WAodeswp WAswp confswp}
%             end
            [WAban, confban]=computeWANoWLvl(resultsdir, ftlcold, gca, b, d, f(it), r(it));
            WAode = hotcold_COLD(b, d, r(it), f(it), rho, stepsize);
            {'NoWlvl' d rho r(it) f(it)  WAode WAban confban}

            for pb=p(2)
               [WAban, confban]=computeWA(resultsdir, ftlcold, gca, b, d, ban, f(it), r(it), pb);
               WAode = hotcold_COLD_ban_mex(b, d, r(it), f(it), rho, pb, stepsize);
               {'BAN' d rho r(it) f(it) pb  WAode WAban confban}
            end
            for ps=p(2)
               [WAswp, confswp]=computeWA(resultsdir, ftlcold, gca, b, d, hcswap, f(it), r(it), ps);
               WAodeswp = hotcold_COLD_hcswap_mex(b, d, r(it), f(it), rho, ps, stepsize);
               {'HCSWAP' d rho r(it) f(it) ps  WAodeswp WAswp confswp}
            end
        end
    end
end

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
function plotWLvl(resultsdir,reffileformat, refleg, ftl, gca, din, wlvl, traceid, fields, pin, savetofile)
    p = cellfun(@(x) num2str(x,'%4.3f'), num2cell(pin'), 'UniformOutput', false);
    leg = cat(1, refleg,  strcat({'p = '}, num2str(pin','%4.3f')));
    d=num2str(din);

    for fieldit = 1:size(fields,1)
        field = fields{fieldit};
        switch field
            case 'fair'
                fieldAxis = 'Fairness';
            case 'end'
                fieldAxis = 'Endurance';
            case 'WA'
                fieldAxis = 'Write Amplification';
            otherwise
                fieldAxis = 'First Cold Block Erase';
        end
        
        reffile = strcat(reffileformat, '-',traceid, '-', field, '.*.csv');
        
        if(savetofile)
            savelocation = strcat(resultsdir,ftl,'trace',wlvl,'-b32-',gca,d,'-sf0.10-f0.010-',traceid, '-', field);
        else
            savelocation='';
        end
        
        pc = strcat(resultsdir,ftl,'trace',wlvl,'/',ftl,'trace',wlvl,'-b32-',gca,d,'-sf0.10-f0.010-p', p, '-',traceid,'-',field,'.*.csv');
        files = cat(1, reffile, pc);
        if(isequal(field,'colderase'))
            barUnmergedFiles(savelocation, files, {'MaxPE',fieldAxis, strcat(ftl, ' ', wlvl,' ', traceid, upper(gca), ' d=',d)}, leg, '', [14 14]);
        else
            plotUnmergedFiles(savelocation, files, {'MaxPE',fieldAxis, strcat(ftl, ' ', wlvl,' ', traceid, upper(gca), ' d=',d)}, leg, '', {}, [14 14]);
        end

%         pc = strcat(ftl,'traceban/',ftl,'traceban-b32-greedy-sf0.10-f0.010-p', p, '-',traceid,'-',field,'.*.csv');
%         files = cat(1, reffile, pc);
%         plotUnmergedFiles(savelocation, files, {'MaxPE',fieldAxis, strcat(traceid, ' Greedy')}, leg, '', {}, [14 14]);
    end
end

function plotWLvlSynth(resultsdir,reffileformat, refleg, ftl, gca, din, wlvl, fin, rin, fields, pin, savetofile)
    p = cellfun(@(x) num2str(x,'%4.3f'), num2cell(pin'), 'UniformOutput', false);
    leg = cat(1, refleg,  strcat({'p = '}, num2str(pin','%4.3f')));
    f = num2str(fin,'%4.3f');
    r = num2str(rin,'%4.3f');
    d = num2str(din);
    
    for fieldit = 1:size(fields,1)
        field = fields{fieldit};
        switch field
            case 'fair'
                fieldAxis = 'Fairness';
            case 'end'
                fieldAxis = 'Endurance';
            %case 'WA'
            otherwise
                fieldAxis = 'Write Amplification';
            %otherwise
            %    fieldAxis = 'First Cold Block Erase';
        end
        reffile = strcat(reffileformat, '-', field, '.*.csv');
        
        if(savetofile)
            savelocation = strcat(resultsdir,ftl,wlvl,'-b32-',gca,d,'-sf0.10-f',f,'-r',r, '-', field);
        else
            savelocation='';
        end
        
        pc = strcat(resultsdir,ftl,wlvl,'/',ftl,wlvl,'-b32-',gca,d,'-sf0.10-f', f, '-r', r, '-p', p,'-',field,'.*.csv');
        files = cat(1, reffile, pc);
        plottitle=strcat(ftl, ' ', wlvl,' f= ',f, ', r= ', r, ', ', upper(gca),' d=',d);
        if(isequal(field,'colderase') || isequal(field,'WA'))
            barUnmergedFiles(savelocation, files, {'MaxPE',fieldAxis, plottitle}, leg, '', [14 14]);
        else
            plotUnmergedFiles(savelocation, files, {'MaxPE',fieldAxis, plottitle}, leg, '', {}, [14 14]);
        end
    end
end

function plot2WLvlSynth(resultsdir,reffileformat, refleg, reffile2format, refleg2, ftl, ftl2, gca, din, wlvl, wlvl2, fin, rin, fields, pin, savetofile)
    p = cellfun(@(x) num2str(x,'%4.3f'), num2cell(pin'), 'UniformOutput', false);
    leg = cat(1, refleg, refleg2,  strcat({strcat(ftl, 'p = ')}, num2str(pin','%4.3f')), strcat({strcat(ftl2, 'p = ')}, num2str(pin','%4.3f')));
    f = num2str(fin,'%4.3f');
    r = num2str(rin,'%4.3f');
    d = num2str(din);
    
    for fieldit = 1:size(fields,1)
        field = fields{fieldit};
        switch field
            case 'fair'
                fieldAxis = 'Fairness';
            case 'end'
                fieldAxis = 'Endurance';
            case 'WA'
                fieldAxis = 'Write Amplification';
            otherwise
                fieldAxis = 'First Cold Block Erase';
        end
        reffile = strcat(reffileformat, '-', field, '.*.csv');
        reffile2 = strcat(reffile2format, '-', field, '.*.csv');
        
        if(savetofile)
            savelocation = strcat(resultsdir,ftl, wlvl,'vs',ftl2,wlvl2,'-b32-',gca,d,'-sf0.10-f',f,'-r',r, '-', field);
        else
            savelocation='';
        end
        
        pc = strcat(resultsdir,ftl,wlvl,'/',ftl,wlvl,'-b32-',gca,d,'-sf0.10-f', f, '-r', r, '-p', p,'-',field,'.*.csv');
        pc2 = strcat(resultsdir,ftl2,wlvl2,'/',ftl2,wlvl2,'-b32-',gca,d,'-sf0.10-f', f, '-r', r, '-p', p,'-',field,'.*.csv');
        files = cat(1, reffile, reffile2, pc, pc2);
        plottitle=strcat(ftl,' ',wlvl,' vs ',ftl2,' ',wlvl2,' f= ',f, ', r= ', r, ', ', upper(gca),' d=',d);
        adaptlim=isequal(field,'fair');
        if(isequal(field,'colderase') || isequal(field,'WA'))
            leg = cat(1, ftl(1:2), ftl(1:2),  strcat({ftl(1)}, num2str(pin','%4.3f')), strcat({ftl2(1)}, num2str(pin','%4.3f')));
            barUnmergedFiles(savelocation, files, {'MaxPE',fieldAxis, plottitle}, leg, '', [14 14], 0);
        else
            plotUnmergedFiles(savelocation, files, {'MaxPE',fieldAxis, plottitle}, leg, '', {}, [14 14],0, adaptlim);
        end
    end
end

function [WA, conf]=computeWANoWLvl(resultsdir, ftl, gca, bin, din, fin, rin)
    b = num2str(bin);
    f = num2str(fin,'%4.3f');
    r = num2str(rin,'%4.3f');
    d = num2str(din);

    field='WA'; 
    %reffile = strcat(reffileformat, '-', field, '.*.csv');
        
        
    pc = strcat(resultsdir,ftl,'/',ftl,'-b',b,'-',gca,d,'-sf0.10-f', f, '-r', r, '-',field,'.*.csv');
%     files = cat(1, reffile, pc);
    [WA,conf]=mean_and_conf_int({pc});
end


function [WA, conf]=computeWA(resultsdir, ftl, gca, bin, din, wlvl, fin, rin, pin)
    p = cellfun(@(x) num2str(x,'%4.3f'), num2cell(pin'), 'UniformOutput', false);
    b = num2str(bin);
    f = num2str(fin,'%4.3f');
    r = num2str(rin,'%4.3f');
    d = num2str(din);

    field='WA'; 
    %reffile = strcat(reffileformat, '-', field, '.*.csv');
        
    pc = strcat(resultsdir,ftl,wlvl,'/',ftl,wlvl,'-b',b,'-',gca,d,'-sf0.10-f', f, '-r', r, '-p', p,'-',field,'.*.csv');
%     files = cat(1, reffile, pc);
    [WA,conf]=mean_and_conf_int(pc);
end
