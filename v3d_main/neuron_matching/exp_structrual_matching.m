function exp_structrual_matching(expnum)

if (expnum ==1) 
    
    a = load(['/Users/longf/work/v3d_2.x/neuron_matching/35_33LHregion_manual.swc'], 'ASCII');
    b = load(['/Users/longf/work/v3d_2.x/neuron_matching/35_33LHregion_auto_root10.swc'], 'ASCII');

    filedir = '/Users/longf/work/v3d_2.x/neuron_matching/exp_result/structure_comp/';
    
    matchingList = load('/Users/longf/work/v3d_2.x/neuron_matching/exp_result/structure_comp_labmeeting/res_exp1.txt', 'ASCII');
    matchingList_allterms = load('/Users/longf/work/v3d_2.x/neuron_matching/exp_result/structure_comp_labmeeting/res_allterms_exp1.txt', 'ASCII');


    display_hierarchical_matching(b, a, matchingList, matchingList_allterms, filedir);

%     %register
% 
%     Pos_subject = b(:,3:5);
% 
%     for i=1:length(matchingList_allterms)
% 
%         ind = find(matchingList_allterms(i,1)==b(:,1));
%         Pos_cpt_subject(i,:) = b(ind,3:5);
% 
%         ind = find(matchingList_allterms(i,2)==a(:,1));
%         Pos_cpt_target(i,:) = a(ind,3:5);
%     end;
% 
%     Pos_subject_new = regPointCloudAffine(Pos_subject, Pos_cpt_subject, Pos_cpt_target);
% 
%     d = b;
%     d(:,3:5) = Pos_subject_new;
% 
%     display_neuron_swcformat_FL(d, 1, [0 0 1]);    
%     display_neuron_swcformat_FL(a, 0, [0 0 0]);
% 
%         
%     saveSwcVariable2File(d, '/Users/longf/work/v3d_2.x/neuron_matching/exp_result/structure_comp/35_33LHregion_auto_root10_aligned.swc');

 
% % registered
%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/35_33LHregion_manual.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/35_33LHregion_auto_align_branching_leaf_root_root10.swc'], 'ASCII');
% 
%     display_neuron_swcformat_FL(b, 1, [0 0 1]);    
%     display_neuron_swcformat_FL(a, 0, [0 0 0]);

end;


% if (expnum ==2) 
%     
%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/jneuron69_root406_pruned_15.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/jneuron68_pruned_15.swc'], 'ASCII');
% 
%    filedir = '/Users/longf/work/v3d_2.x/neuron_matching/';
% 
%     matchingList = load('/Users/longf/work/v3d_2.x/neuron_matching/res.txt', 'ASCII');
%     matchingList_allterms = load('/Users/longf/work/v3d_2.x/neuron_matching/res.txt_allterms.txt', 'ASCII');
%     
%     display_hierarchical_matching(b, a, matchingList, matchingList_allterms, filedir);
% 
% 
% 
% end;



if (expnum ==2) 
    
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/35_33LHregion_auto_align_branching_leaf_root_root10.swc'], 'ASCII');    
%     b(:,5) = b(:,5)+40;
%     outfilename = ['/Users/longf/work/v3d_2.x/neuron_matching/35_33LHregion_auto_align_branching_leaf_root_root10_shift40.swc'];
%     saveSwcVariable2File(b, outfilename);

    
    a = load(['/Users/longf/work/v3d_2.x/neuron_matching/exp_result/structure_comp_jfrc_bioimaging_4gene/35_33LHregion_manual.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/35_33LHregion_auto_align_branching_leaf_root_root10.swc'], 'ASCII');
    b = load(['/Users/longf/work/v3d_2.x/neuron_matching/exp_result/structure_comp_jfrc_bioimaging_4gene/35_33LHregion_auto_align_branching_leaf_root_root10_shift40.swc'], 'ASCII');
        
    filedir = '/Users/longf/work/v3d_2.x/neuron_matching/exp_result/structure_comp_jfrc_bioimaging_4gene/';
    
    matchingList = load('/Users/longf/work/v3d_2.x/neuron_matching/exp_result/structure_comp_jfrc_bioimaging_4gene/res.txt', 'ASCII');
    matchingList_allterms = load('/Users/longf/work/v3d_2.x/neuron_matching/exp_result/structure_comp_jfrc_bioimaging_4gene/res.txt_allterms.txt', 'ASCII');

%     display_hierarchical_matching(b, a, matchingList, matchingList_allterms, filedir);
    gen_swc4v3d_hierarchical_matching(b, a, matchingList_allterms, filedir);
end;


if (expnum ==3) 
    
    
    a = load(['/Users/longf/work/v3d_2.x/neuron_matching/35_33LHregion_manual.swc'], 'ASCII');
    b = load(['/Users/longf/work/v3d_2.x/neuron_matching/35_33LHregion_auto_align_branching_leaf_root_root10_shift40.swc'], 'ASCII');
        
    filedir = '/Users/longf/work/v3d_2.x/neuron_matching/';
    
    matchingList = load('/Users/longf/work/v3d_2.x/neuron_matching/res.txt', 'ASCII');
    matchingList_allterms = load('/Users/longf/work/v3d_2.x/neuron_matching/res.txt_allterms.txt', 'ASCII');

    gen_swc4v3d_hierarchical_matching(b, a, matchingList_allterms, filedir);
end;


if (expnum ==4) 
    
%     shift the z coordinate of the first tree, and use the shifted neuron
%     to do comparison

%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/two_trials_zeroed/trace_1_zero/15_13LHregion0.ASC.swc'], 'ASCII');    
%     b(:,5) = b(:,5)+40;
%     outfilename = ['/Users/longf/work/v3d_2.x/neuron_matching/data/two_trials_zeroed/trace_1_zero/15_13LHregion0_shift40.ASC.swc'];
%     saveSwcVariable2File(b, outfilename);
% 
    
%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/two_trials_zeroed/trace_2_zero/15_13LHregion0_b.ASC.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/two_trials_zeroed/trace_1_zero/15_13LHregion0_shift40.ASC.swc'], 'ASCII');        
%     filedir = '/Users/longf/work/v3d_2.x/neuron_matching/data/result/15_13LHregion0/';

%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/two_trials_zeroed/trace_2_zero/63_61LHregion_b.ASC.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/two_trials_zeroed/trace_1_zero/63_61LHregion_shift40.ASC.swc'], 'ASCII');        
%     filedir = '/Users/longf/work/v3d_2.x/neuron_matching/data/result/63_61LHregion/';

%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/two_trials_zeroed/trace_2_zero/59_57LHregion_b.ASC.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/two_trials_zeroed/trace_1_zero/59_57LHregion_shift40.ASC.swc'], 'ASCII');        
%     filedir = '/Users/longf/work/v3d_2.x/neuron_matching/data/result/59_57LHregion/';

%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/two_trials_zeroed/trace_2_zero/55_53LHregion_b.ASC.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/two_trials_zeroed/trace_1_zero/55_53LHregion_shift40.ASC.swc'], 'ASCII');        
%     filedir = '/Users/longf/work/v3d_2.x/neuron_matching/data/result/55_53LHregion/';

%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/two_trials_zeroed/trace_2_zero/51_49LHregion_b.ASC.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/two_trials_zeroed/trace_1_zero/51_49LHregion_shift40.ASC.swc'], 'ASCII');        
% %     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/two_trials_zeroed/trace_1_zero/51_49LHregion.ASC.swc'], 'ASCII');        
%     filedir = '/Users/longf/work/v3d_2.x/neuron_matching/data/result/51_49LHregion/';

%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/two_trials_zeroed/trace_2_zero/51_49LHregion_b.ASC.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/two_trials_zeroed/trace_1_zero/51_49LHregion.ASC.swc'], 'ASCII');        
%     filedir = '/Users/longf/work/v3d_2.x/neuron_matching/data/result/51_49LHregion/';

%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/two_trials_zeroed/trace_2_zero/47_45LHregion_b.ASC.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/two_trials_zeroed/trace_1_zero/47_45LHregion_shift40.ASC.swc'], 'ASCII');        
%     filedir = '/Users/longf/work/v3d_2.x/neuron_matching/data/result/47_45LHregion/';

%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/two_trials_zeroed/trace_2_zero/43_41LHregion_b.ASC.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/two_trials_zeroed/trace_1_zero/43_41LHregion_shift40.ASC.swc'], 'ASCII');        
%     filedir = '/Users/longf/work/v3d_2.x/neuron_matching/data/result/43_41LHregion/';
% 

%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/two_trials_zeroed/trace_2_zero/31_29LHregion0_b.ASC.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/two_trials_zeroed/trace_1_zero/31_29LHregion0_shift40.ASC.swc'], 'ASCII');        
%     filedir = '/Users/longf/work/v3d_2.x/neuron_matching/data/result/31_29LHregion0/';
% 
% 
%   recomb paper, 20090930

%   make neuron thicker for display purpose
%     a(:,6) = a(:,6)*3;
%     b(:,6) = b(:,6)*3;
%     
%     saveSwcVariable2File(a, ['/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/51_49LHregion/51_49LHregion_b_3x.ASC.swc']);
%     saveSwcVariable2File(b, ['/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/51_49LHregion/51_49LHregion_3x.ASC.swc']);

    
    
%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/31_29LHregion0/31_29LHregion0_b_3x.ASC.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/31_29LHregion0/31_29LHregion0_3x_rotate_xn5.ASC.swc'], 'ASCII');        
%     filedir = '/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/31_29LHregion0/';

%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/47_45LHregion/47_45LHregion_b_3x.ASC.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/47_45LHregion/47_45LHregion_3x_rotate_z5.ASC.swc'], 'ASCII');        
%     filedir = '/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/47_45LHregion/';


%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/31_29LHregion0_3x.ASC.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/31_29LHregion0_3x_shiftz40.ASC.swc'], 'ASCII');        
%     filedir = '/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/rate0.0_dis0.0/';
 
%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/31_29LHregion0_3x.ASC.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/31_29LHregion0_3x.ASC.swc_perturbed_rate0.1_dis5.0_shiftz40.swc'], 'ASCII');        
%     filedir = '/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/rate0.1_dis5.0/';

%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/31_29LHregion0_3x.ASC.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/31_29LHregion0_3x.ASC.swc_perturbed_rate0.1_dis10.0_shiftz40.swc'], 'ASCII');        
%     filedir = '/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/rate0.1_dis10.0/';
% 
%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/31_29LHregion0_3x.ASC.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/31_29LHregion0_3x.ASC.swc_perturbed_rate0.1_dis15.0_shiftz40.swc'], 'ASCII');        
%     filedir = '/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/rate0.1_dis15.0/';
% 
%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/31_29LHregion0_3x.ASC.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/31_29LHregion0_3x.ASC.swc_perturbed_rate0.3_dis5.0_shiftz40.swc'], 'ASCII');        
%     filedir = '/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/rate0.3_dis5.0/';
% 
%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/31_29LHregion0_3x.ASC.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/31_29LHregion0_3x.ASC.swc_perturbed_rate0.3_dis10.0_shiftz40.swc'], 'ASCII');        
%     filedir = '/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/rate0.3_dis10.0/';
% 
%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/31_29LHregion0_3x.ASC.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/31_29LHregion0_3x.ASC.swc_perturbed_rate0.3_dis15.0_shiftz40.swc'], 'ASCII');        
%     filedir = '/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/rate0.3_dis15.0/';
%    
%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/31_29LHregion0_3x.ASC.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/31_29LHregion0_3x.ASC.swc_perturbed_rate0.5_dis5.0_shiftz40.swc'], 'ASCII');        
%     filedir = '/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/rate0.5_dis5.0/';
% 
%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/31_29LHregion0_3x.ASC.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/31_29LHregion0_3x.ASC.swc_perturbed_rate0.5_dis10.0_shiftz40.swc'], 'ASCII');        
%     filedir = '/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/rate0.5_dis10.0/';
% 
%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/31_29LHregion0_3x.ASC.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/31_29LHregion0_3x.ASC.swc_perturbed_rate0.5_dis15.0_shiftz40.swc'], 'ASCII');        
%     filedir = '/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/rate0.5_dis15.0/';
%     

%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/31_29LHregion0_3x.ASC.swc'], 'ASCII');
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/31_29LHregion0_3x.ASC.swc_perturbed_rate0.5_dis15.0_shiftz40.swc'], 'ASCII');        
%     filedir = '/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/perturbation/31_29LHregion0/rate0.5_dis15.0/';
% 

%     a = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/deletion/51_49LHregion/51_49LHregion_b_3x.ASC.swc_delete.swc'], 'ASCII');
% %     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/deletion/51_49LHregion/51_49LHregion_3x_shiftz40.ASC.swc'], 'ASCII');        
% 
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/deletion/51_49LHregion/51_49LHregion_3x_shiftz40.ASC.swc_delete.swc'], 'ASCII');        
%     filedir = '/Users/longf/work/v3d_2.x/neuron_matching/data/result/recomb_paper/deletion/51_49LHregion/';
    

%     a = load(['/Users/longf/Desktop/2.swc'], 'ASCII');
% %     b = a;
% %     b(:,5) = a(:,5)+40;
% %     outfilename = ['/Users/longf/Desktop/2.swc'];
% %     saveSwcVariable2File(b, outfilename);    
%     b = load(['/Users/longf/Desktop/1.swc'], 'ASCII');
%     filedir = '/Users/longf/Desktop/1/';

    a = load(['/Users/longf/work/v3d_2.0/neuron_matching/data/two_trials_zeroed/trace_2_zero/51_49LHregion_b.ASC.swc'], 'ASCII');
    b = load(['/Users/longf/work/v3d_2.0/neuron_matching/data/two_trials_zeroed/trace_1_zero/51_49LHregion_shift40.ASC.swc'], 'ASCII');        
%     b = load(['/Users/longf/work/v3d_2.x/neuron_matching/data/two_trials_zeroed/trace_1_zero/51_49LHregion.ASC.swc'], 'ASCII');        
    filedir = '/Users/longf/work/v3d_2.0/neuron_matching/data/result/51_49LHregion_20100128/';

    matchingList = load([filedir, 'res.txt'], 'ASCII');
    matchingList_allterms = load([filedir, 'res_allterms.txt'], 'ASCII');

    gen_swc4v3d_hierarchical_matching(b, a, matchingList_allterms, filedir);
end;
