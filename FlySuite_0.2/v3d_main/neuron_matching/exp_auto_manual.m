function exp_auto_manual(expnum)

% -----------------------------------------------------
% match neurons with automatic and manual reconstruction
% ------------------------------------------------------

% matching significant branching points in two trees

% run -I 35_33LHregion_auto.swc -i 35_33LHregion_manual.swc -o res.txt

tmp = '35_33';

a = load(['/Users/longf/work/PHC_work/neuronmorphometry/res/hanchuan_rec/Neurolucida_reconstructed_neurons/swc_z_rescaled/c_', tmp, 'LHregion.ASC.swc'], 'ASCII');
b = load(['/Users/longf/work/PHC_work/neuronmorphometry/res/hanchuan_rec/pc_swc/', tmp, 'LHregion.lsm.pc.swc'], 'ASCII');

% aa = a;
% aa(:,6) = aa(:,6)/max(aa(:,6));
% % aa = round(aa);
% % save /Users/longf/work/v3d_2.x/neuron_matching/exp_result/auto_manual/35_33LHregion_manual.swc aa -ASCII;
% 
% saveSwcVariable2File(aa, '/Users/longf/work/v3d_2.x/neuron_matching/exp_result/auto_manual/35_33LHregion_manual.swc');



if (expnum==1) % display two neurons
    display_neuron_swcformat_FL(a, 1, [0 0 0]);
    display_neuron_swcformat_FL(b, 0, [0 0 1]);
end;


if (expnum ==2) % display significant branching points and matching
    
    display_neuron_swcformat_FL(a, 1, [0 0 0]);

    c = load('/Users/longf/work/v3d_2.x/neuron_matching/exp_result/auto_manual/sigPoints2_branching','ASCII');

    hold on;
    for i=1:length(c)
        plot3(a(c(i),3), a(c(i),4), a(c(i),5),'o','color', [1,0,0]); 
    end;


    display_neuron_swcformat_FL(b, 0, [0 0 1]);

    c = load('/Users/longf/work/v3d_2.x/neuron_matching/exp_result/auto_manual/sigPoints1_branching','ASCII');
    hold on;
    for i=1:length(c)
        plot3(b(c(i),3), b(c(i),4), b(c(i),5),'o','color', [1,0,0]); 
    end;

    matchingList = load('/Users/longf/work/v3d_2.x/neuron_matching/exp_result/auto_manual/res_auto_manual.txt', 'ASCII');

    display_neuron_match(b, a, matchingList);
    
    % register
    idx = [1:length(matchingList)];
    
    Pos_subject = b(:,3:5);
    Pos_cpt_subject = b(matchingList(idx,1),3:5);
    Pos_cpt_target = a(matchingList(idx,2),3:5);

    Pos_subject_new = regPointCloudAffine(Pos_subject, Pos_cpt_subject, Pos_cpt_target);

    d = b;
    d(:,3:5) = Pos_subject_new;

    display_neuron_swcformat_FL(a, 1, [0 0 0]);
    display_neuron_swcformat_FL(d, 0, [0 0 1]);

    hold on;
    for i=1:length(matchingList)
        plot3(a(matchingList(i,2),3), a(matchingList(i,2),4), a(matchingList(i,2),5),'o','color', [1,0,0]); 
    end;


    for i=1:length(matchingList)
        plot3(d(matchingList(i,1),3), d(matchingList(i,1),4), d(matchingList(i,1),5),'o','color', [0,1,0]); 
    end;
    
    saveSwcVariable2File(d, '/Users/longf/work/v3d_2.x/neuron_matching/exp_result/auto_manual/35_33LHregion_auto_align_branching.swc');

    
end;



if (expnum ==3) % display significant branching points + significant leaf nodes and matching, and register
    
    display_neuron_swcformat_FL(a, 1, [0 0 0]);

    c = load('/Users/longf/work/v3d_2.x/neuron_matching/exp_result/auto_manual/sigPoints2_branching_leaf_root','ASCII');
    matchingList = load('/Users/longf/work/v3d_2.x/neuron_matching/exp_result/auto_manual/res_auto_manual2.txt', 'ASCII');

    c = matchingList(:,2);
    hold on;
    for i=1:length(c)
        plot3(a(c(i),3), a(c(i),4), a(c(i),5),'o','color', [1,0,0]); 
    end;


    display_neuron_swcformat_FL(b, 0, [0 0 1]);

    c = load('/Users/longf/work/v3d_2.x/neuron_matching/exp_result/auto_manual/sigPoints1_branching_leaf_root','ASCII');
    c = matchingList(:,1);

    hold on;
    for i=1:length(c)
        plot3(b(c(i),3), b(c(i),4), b(c(i),5),'o','color', [0,1,0]); 
    end;

    matchingList = load('/Users/longf/work/v3d_2.x/neuron_matching/exp_result/auto_manual/res_auto_manual2.txt', 'ASCII');

    idx = [1,2,5,6,7,8,9,11,12,14];
%     idx = [1,2,3,4,5,6,7,8,9,11,12,14];
 
%     display_neuron_match(b, a, matchingList(idx,:));
    display_neuron_match(b, a, matchingList);
    
    % register
    
    Pos_subject = b(:,3:5);
    Pos_cpt_subject = b(matchingList(idx,1),3:5);
    Pos_cpt_target = a(matchingList(idx,2),3:5);

    Pos_subject_new = regPointCloudAffine(Pos_subject, Pos_cpt_subject, Pos_cpt_target);

    d = b;
    d(:,3:5) = Pos_subject_new;

    display_neuron_swcformat_FL(a, 1, [0 0 0]);
    display_neuron_swcformat_FL(d, 0, [0 0 1]);

    hold on;
    for i=1:length(matchingList)
        plot3(a(matchingList(i,2),3), a(matchingList(i,2),4), a(matchingList(i,2),5),'o','color', [1,0,0]); 
    end;


    for i=1:length(matchingList)
        plot3(d(matchingList(i,1),3), d(matchingList(i,1),4), d(matchingList(i,1),5),'o','color', [0,1,0]); 
    end;
    
    saveSwcVariable2File(d, '/Users/longf/work/v3d_2.x/neuron_matching/exp_result/auto_manual/35_33LHregion_auto_align_branching_leaf_root.swc');


end;


if (expnum ==4) %test purpose, very important

    display_neuron_swcformat_FL(a, 1, [0 0 0]);

    c = load('/Users/longf/work/v3d_2.x/neuron_matching/exp_result/auto_manual/sigPoints2_branching_leaf_root','ASCII');
    matchingList = load('/Users/longf/work/v3d_2.x/neuron_matching/exp_result/auto_manual/res_auto_manual2.txt', 'ASCII');

    hold on;
    for i=1:length(c)
        plot3(a(c(i),3), a(c(i),4), a(c(i),5),'o','color', [1,0,0]); 
    end;


    display_neuron_swcformat_FL(b, 0, [0 0 1]);

    c = load('/Users/longf/work/v3d_2.x/neuron_matching/exp_result/auto_manual/sigPoints1_branching_leaf_root','ASCII');

    hold on;
    for i=1:length(c)
        plot3(b(c(i),3), b(c(i),4), b(c(i),5),'o','color', [0,1,0]); 
    end;


    
    % multiple run, test which idx is good
    
    num = 7;
    tag = zeros(1,length(matchingList));

    cnt = 1;
    while(sum(tag)<num)

        while(1)

            idx(cnt) = min(max(round(rand()*length(matchingList)),1), length(matchingList));

            if tag(idx(cnt))==0
                tag(idx(cnt)) = 1;
                cnt = cnt+1;
                break;
            end;

        end;
    end;

    idx = union(idx,1)
%     idx = setdiff(idx,3)

    % idx = [1,2,5,6,7,8,9,11,12,14];




    Pos_subject = b(:,3:5);
    Pos_cpt_subject = b(matchingList(idx,1),3:5);
    Pos_cpt_target = a(matchingList(idx,2),3:5);

    Pos_subject_new = regPointCloudAffine(Pos_subject, Pos_cpt_subject, Pos_cpt_target);

    d = b;
    d(:,3:5) = Pos_subject_new;

    display_neuron_swcformat_FL(a, 0, [0 0 0]);
    display_neuron_swcformat_FL(d, 0, [0 0 1]);

    hold on;
    for i=1:length(matchingList)
        plot3(a(matchingList(i,2),3), a(matchingList(i,2),4), a(matchingList(i,2),5),'o','color', [1,0,0]); 
    end;


    for i=1:length(matchingList)
        plot3(d(matchingList(i,1),3), d(matchingList(i,1),4), d(matchingList(i,1),5),'o','color', [0,1,0]); 
    end;
end;


if (expnum ==5) 
    
   a = load(['/Users/longf/work/v3d_2.x/neuron_matching/exp_result/auto_manual/35_33LHregion_manual.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/exp_result/auto_manual/35_33LHregion_auto_align_branching_leaf_root.swc'], 'ASCII');
    b = load(['/Users/longf/work/v3d_2.x/neuron_matching/exp_result/auto_manual/35_33LHregion_auto_align_branching_leaf_root_root10.swc'], 'ASCII');


%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/exp_result/auto_manual/35_33LHregion_auto_align_branching_leaf_root.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/exp_result/auto_manual/35_33LHregion_auto_align_branching_leaf_root_root10.swc'], 'ASCII');

%    b(:,4) = b(:,4) + 20;
    
    display_neuron_swcformat_FL(a, 1, [0 0 0]);
    display_neuron_swcformat_FL(b, 0, [0 0 1]);
    
    hold on;
    
    plot3(a(123,3), a(123,4), a(123,5),'d', 'color', [1,0,0]);
    plot3(b(147,3), b(147,4), b(147,5),'d', 'color', [1,0,0]);
    
% %     matchingList = load('/Users/longf/work/v3d_2.x/neuron_matching/exp_result/auto_manual/res_auto_manual3.txt', 'ASCII');
%     matchingList = load('/Users/longf/work/v3d_2.x/neuron_matching/exp_result/auto_manual/res.txt', 'ASCII');
% 
%     display_neuron_match(b, a, matchingList);
end;


if (expnum ==6) % % pruned and remove continual points
    
    a = load(['/Users/longf/work/v3d_2.x/neuron_matching/exp_result/auto_manual/5.swc'], 'ASCII');
    b = load(['/Users/longf/work/v3d_2.x/neuron_matching/exp_result/auto_manual/4.swc'], 'ASCII');

%    b(:,4) = b(:,4) + 20;

     b(:,5) = b(:,5) + 20;
    
%     display_neuron_swcformat_FL(a, 1, [0 0 0]); 
%     display_neuron_swcformat_FL(b, 0, [0 0 1]);
    
    hold on;
    
    
    matchingList = load('/Users/longf/work/v3d_2.x/neuron_matching/exp_result/auto_manual/res_4_5.txt', 'ASCII');
%     matchingList = load('/Users/longf/work/v3d_2.x/neuron_matching/res.txt', 'ASCII');

%     display_neuron_match(b, a, matchingList);

    display_neuron_match_nonmatch2(b, a, matchingList, [0 0 1], [0 0 0], [0 1 1], [1 0 0]);

end;

if (expnum ==7) % full match
    
    a = load(['/Users/longf/work/v3d_2.x/neuron_matching/exp_result/auto_manual/35_33LHregion_manual.swc'], 'ASCII');
    b = load(['/Users/longf/work/v3d_2.x/neuron_matching/exp_result/auto_manual/35_33LHregion_auto_align_branching_leaf_root_root10.swc'], 'ASCII');


     b(:,5) = b(:,5) + 20;
    
%     display_neuron_swcformat_FL(a, 1, [0 0 0]); 
%     display_neuron_swcformat_FL(b, 0, [0 0 1]);
    
    hold on;
    
    
%     matchingList = load('/Users/longf/work/v3d_2.x/neuron_matching/exp_result/auto_manual/res_35_33LHregion_manual_auto.txt', 'ASCII');
    matchingList = load('/Users/longf/work/v3d_2.x/neuron_matching/exp_result/auto_manual/res_4_5.txt', 'ASCII');

%     display_neuron_match(b, a, matchingList);

    display_neuron_match_nonmatch2(b, a, matchingList, [0 0 1], [0 0 0], [0 1 1], [1 0 0]);

end;
