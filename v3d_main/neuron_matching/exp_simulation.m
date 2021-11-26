function exp_simulation(expnum)
% -----------------------------
% match neurons with perturbation
% ------------------------------

%run -I testreal3.swc -i testreal.swc -o res.txt

% a = load('/Users/longf/work/v3d_2.x/neuron_matching/testreal.swc', 'ASCII');
% b = load('/Users/longf/work/v3d_2.x/neuron_matching/testreal3.swc', 'ASCII');
% 
% b(:,4) = b(:,4)+120;
% saveSwcVariable2File(a, '/Users/longf/work/v3d_2.x/neuron_matching/exp_result/simulation/testreal.swc');
% saveSwcVariable2File(b, '/Users/longf/work/v3d_2.x/neuron_matching/exp_result/simulation/testreal_deletion_20_0.7_203deleted.swc');
% saveSwcVariable2File(b, '/Users/longf/work/v3d_2.x/neuron_matching/exp_result/simulation/testreal_deletion_10_0.5_145deleted.swc');
% saveSwcVariable2File(b, '/Users/longf/work/v3d_2.x/neuron_matching/exp_result/simulation/testreal_deletion_80_0.2_58deleted.swc');


if (expnum==1) %random deletion
    
    a = load('/Users/longf/work/v3d_2.x/neuron_matching/exp_result/simulation/testreal.swc', 'ASCII');
    b = load('/Users/longf/work/v3d_2.x/neuron_matching/exp_result/simulation/testreal_deletion_20_0.7_203deleted.swc', 'ASCII');

    b(:,4) = b(:,4)+20;
    display_neuron_swcformat_FL(a, 1, [0 0 0]);
    display_neuron_swcformat_FL(b, 0, [0 0 1]);

    matchingList = load('/Users/longf/work/v3d_2.x/neuron_matching/exp_result/simulation/res_deletion_20_0.7.txt', 'ASCII');
    display_neuron_match(b, a, matchingList);

end;

if (expnum==2) %coordinate perturbation
    
    a = load('/Users/longf/work/v3d_2.x/neuron_matching/exp_result/simulation/testreal.swc', 'ASCII');
    b = load('/Users/longf/work/v3d_2.x/neuron_matching/exp_result/simulation/testreal_coorperturb_0.1_3_29perturbed.swc', 'ASCII');

    b(:,4) = b(:,4)+20;
    display_neuron_swcformat_FL(a, 1, [0 0 0]);
    display_neuron_swcformat_FL(b, 0, [0 0 1]);

    matchingList = load('/Users/longf/work/v3d_2.x/neuron_matching/exp_result/simulation/res_coorperturb_0.1_3.txt', 'ASCII');
    display_neuron_match(b, a, matchingList);
end;

if (expnum==3) %coordinate perturbation
    
    a = load('/Users/longf/work/v3d_2.x/neuron_matching/exp_result/simulation/testreal.swc', 'ASCII');
    b = load('/Users/longf/work/v3d_2.x/neuron_matching/exp_result/simulation/testreal_deletion_80_0.2_coorpertub_1_2.swc', 'ASCII');

    b(:,4) = b(:,4)+20;
    display_neuron_swcformat_FL(a, 1, [0 0 0]);
    display_neuron_swcformat_FL(b, 0, [0 0 1]);

    matchingList = load('/Users/longf/work/v3d_2.x/neuron_matching/exp_result/simulation/res_deletion_80_0.2_coorpertub_1_2.txt', 'ASCII');
    display_neuron_match(b, a, matchingList);
end;