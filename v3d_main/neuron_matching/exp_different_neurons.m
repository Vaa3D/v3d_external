function exp_different_neurons(expnum)
% match of neurons of different structures
%
% check which two are similar, goto:
% /Users/longf/work/PHC_work/neuronmorphometry
% use: batch_plot_dendrogram_greg_data
% identify the leaves in the dendrogram

%---------------
% prepare data
%---------------

% load('/Users/longf/work/PHC_work/neuronmorphometry/data/data_jefferies.mat');
% saveSwcVariable2File(jneuron{9}, '/Users/longf/work/v3d_2.x/neuron_matching/jneuron9.swc');
% saveSwcVariable2File(jneuron{10}, '/Users/longf/work/v3d_2.x/neuron_matching/jneuron10.swc');
% 
% saveSwcVariable2File(jneuron{68}, '/Users/longf/work/v3d_2.x/neuron_matching/jneuron68.swc');
% saveSwcVariable2File(jneuron{69}, '/Users/longf/work/v3d_2.x/neuron_matching/jneuron69.swc');

% close all
% 
% for i = 6:10
% a = jneuron{i}; 
% display_neuron_swcformat_FL(a, 1, [0 0 1]);
% end;
% 
% idx1 = 9;
% idx2 = 10;
% 
% % idx1 = 127;
% % idx2 = 164;
% % 
% % idx1 = 204;
% % idx2 = 210;
% % 
% 
% a = jneuron{idx1};
% display_neuron_swcformat_FL(a, 1, [1 0 0]);
% a = jneuron{idx2};
% display_neuron_swcformat_FL(a, 1, [0 0 1]);


% for i = 10:50
%     for j=1:5
%         (i-1)*5+j
%         a = jneuron{(i-1)*5+j}; 
%         display_neuron_swcformat_FL(a, 1, [0 0 1]);
%     end;
%     pause;
%     close all
% end;

if (expnum ==1) 
    
    a = load(['/Users/longf/work/v3d_2.x/neuron_matching/exp_result/different_neurons/diffneuron2.swc'], 'ASCII');
    b = load(['/Users/longf/work/v3d_2.x/neuron_matching/exp_result/different_neurons/diffneuron1.swc'], 'ASCII');

%    b(:,4) = b(:,4) + 20;

     b(:,5) = b(:,5) + 20;
    
%     display_neuron_swcformat_FL(a, 1, [0 0 0]); 
%     display_neuron_swcformat_FL(b, 0, [0 0 1]);
%     
%     hold on;
    
    
    matchingList = load('/Users/longf/work/v3d_2.x/neuron_matching/exp_result/different_neurons/res_diffneurons_1_2_struct.txt', 'ASCII');
%     display_neuron_match(b, a, matchingList);

    display_neuron_match_nonmatch(b, a, matchingList, [0 0 1], [0 0 0], [0 1 1], [1 0 0]);

    idx = find(matchingList(:,2)>0);

    % register
    
    Pos_subject = b(:,3:5);
    
    for i=1:length(idx)
        
        ind = find(matchingList(idx(i),1)==b(:,1));
        Pos_cpt_subject(i,:) = b(ind,3:5);

        ind = find(matchingList(idx(i),2)==a(:,1));
        Pos_cpt_target(i,:) = a(ind,3:5);
    end;

    Pos_subject_new = regPointCloudAffine(Pos_subject, Pos_cpt_subject, Pos_cpt_target);

    d = b;
    d(:,3:5) = Pos_subject_new;

    display_neuron_swcformat_FL(d, 1, [0 0 1]);    
    display_neuron_swcformat_FL(a, 0, [0 0 0]);


    saveSwcVariable2File(d, '/Users/longf/work/v3d_2.x/neuron_matching/exp_result/different_neurons/diffneuron1_aligned.swc');



end;


if (expnum ==2) 
    
    a = load(['/Users/longf/work/v3d_2.x/neuron_matching/exp_result/different_neurons/diffneuron2.swc'], 'ASCII');
    b = load(['/Users/longf/work/v3d_2.x/neuron_matching/exp_result/different_neurons/diffneuron1_aligned.swc'], 'ASCII');

%    b(:,4) = b(:,4) + 20;

     b(:,5) = b(:,5) + 20;
    
%     display_neuron_swcformat_FL(a, 1, [0 0 0]); 
%     display_neuron_swcformat_FL(b, 0, [0 0 1]);
%     
%     hold on;
%     
%     
    matchingList = load('/Users/longf/work/v3d_2.x/neuron_matching/res_diffneurons_1aligned_2_dist.txt', 'ASCII');
%     display_neuron_match(b, a, matchingList);

    figure;
    display_neuron_match_nonmatch2(b, a, matchingList, [0 0 1], [0 0 0], [0 1 1], [1 0 0]);


end;


if (expnum ==3) 
    
%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/exp_result/different_neurons/jneuron10_root185.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/exp_result/different_neurons/jneuron9.swc'], 'ASCII');

%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/jneuron69_root406.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/jneuron68.swc'], 'ASCII');

%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/jneuron69_root406_pruned.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/jneuron68_pruned.swc'], 'ASCII');

%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/jneuron69_root406_pruned_30.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/jneuron68_pruned_30.swc'], 'ASCII');
% 
%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/jneuron69_root406_pruned_20.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/jneuron68_pruned_20.swc'], 'ASCII');
%  
%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/jneuron69_root406_pruned_15.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/jneuron68_pruned_15.swc'], 'ASCII');
% 
%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/jneuron69_root406_pruned_10.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/jneuron68_pruned_10.swc'], 'ASCII');
% 

%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/t2.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/t1.swc'], 'ASCII');
% 
%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/5.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/4.swc'], 'ASCII');
% 
% 
%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/35_33LHregion_manual.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/35_33LHregion_auto_align_branching_leaf_root_root10.swc'], 'ASCII');
% 
    a = load(['/Users/longf/work/v3d_2.x/neuron_matching/35_33LHregion_manual.swc'], 'ASCII');
    b = load(['/Users/longf/work/v3d_2.x/neuron_matching/35_33LHregion_auto_root10.swc'], 'ASCII');


%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/tree2.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/tree1.swc'], 'ASCII');


% 
%    b(:,4) = b(:,4) + 20;

%      b(:,5) = b(:,5) + 20;
    
%     display_neuron_swcformat_FL(a, 1, [0 0 0]); 
%     display_neuron_swcformat_FL(b, 0, [0 0 1]);
%     
%     hold on;
%     
%     
    matchingList = load('/Users/longf/work/v3d_2.x/neuron_matching/res.txt', 'ASCII');
    matchingList_allterms = load('/Users/longf/work/v3d_2.x/neuron_matching/res.txt_allterms.txt', 'ASCII');
    
%     display_neuron_match(b, a, matchingList);

%     figure;
% %      display_neuron_match_nonmatch2(b, a, matchingList, [0 0 1], [0 0 0], [0 1 1], [1 0 0]);
%      display_neuron_match_nonmatch2(b, a, matchingList, [0 0 1], [0 0 0], [0 0 1], [0 0 0]);
     display_hierarchical_matching(b, a, matchingList, matchingList_allterms);

%         % register
%     iter = 0;
%     while (iter<10)
% 
%         iter = iter+1;
%         % multiple run, test which idx is good
% 
%         num = 7;
%         idxpool = find(matchingList(:,2)>0);
%         tag = zeros(1,length(idxpool));
% 
%         cnt = 1;
%         idx = [];
%         while(sum(tag)<num)
% 
%             while(1)
% 
%                 idx(cnt) = min(max(round(rand()*length(idxpool)),1), length(idxpool));
% 
%                 if tag(idx(cnt))==0
%                     tag(idx(cnt)) = 1;
%                     cnt = cnt+1;
%                     break;
%                 end;
% 
%             end;
%         end;
% 
%         idx = union(idxpool(idx),1)
% 
% 
%     %     idx = find(matchingList(:,2)>0);
% 
% 
%         Pos_subject = b(:,3:5);
% 
%         for i=1:length(idx)
% 
%             ind = find(matchingList(idx(i),1)==b(:,1));
%             Pos_cpt_subject(i,:) = b(ind,3:5);
% 
%             ind = find(matchingList(idx(i),2)==a(:,1));
%             Pos_cpt_target(i,:) = a(ind,3:5);
%         end;
% 
%         Pos_subject_new = regPointCloudAffine(Pos_subject, Pos_cpt_subject, Pos_cpt_target);
% 
%         d = b;
%         d(:,3:5) = Pos_subject_new;
% 
%         display_neuron_swcformat_FL(d, 1, [0 0 1]);    
%         display_neuron_swcformat_FL(a, 0, [0 0 0]);
% 
%     end;
%     saveSwcVariable2File(d, '/Users/longf/work/v3d_2.x/neuron_matching/exp_result/different_neurons/jneuron9_aligned.swc');


    
end;


