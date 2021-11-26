% -----------------------------
% match neurons with perturbation
% ------------------------------

% load('/Users/longf/work/PHC_work/neuronmorphometry/data/data_jefferies.mat');
% a = jneuron{1}; 
% b = load('/Users/longf/work/v3d_2.x/neuron_matching/testreal3.swc', 'ASCII');
% 
% % display_neuron_swcformat_FL(a);
% % display_neuron_swcformat_FL(b, 0, [0 0 1]);


% a = load('/Users/longf/work/v3d_2.x/neuron_matching/toy.swc', 'ASCII');
% b = load('/Users/longf/work/v3d_2.x/neuron_matching/toy1.swc', 'ASCII');

% a = load('/Users/longf/work/v3d_2.x/neuron_matching/testreal.swc', 'ASCII');
% b = load('/Users/longf/work/v3d_2.x/neuron_matching/testreal3.swc', 'ASCII');

% b(:,4) = b(:,4)+20;
% display_neuron_swcformat_FL(a, 1, [0 0 0]);
% display_neuron_swcformat_FL(b, 0, [0 0 1]);

% matchingList = load('/Users/longf/work/v3d_2.x/neuron_matching/res.txt', 'ASCII');
% display_neuron_match(b, a, matchingList);

% --------------------------------------------------------------
% test the shortest distance between two nodes in testreal.swc
% --------------------------------------------------------------

% tt = sqrt(dist2(b(:,3:5), b(:,3:5)));
% min(tt(tt>0))
% 
% % number of nodes with the shortest distance with other nodes shorter than
% % 3, or 1, or 0.5
% 
% tt2 = tt;
% tt2(find(tt==0)) = 999;
% [Y,I] = min(tt2, [], 2);
% 
% nnz(Y<(3*sqrt(3)/2))
% nnz(Y<(3*sqrt(0.5)/2))
% nnz(Y<(3*sqrt(0.1)/2))

% -----------------------------------------------------
% match neurons with automatic and manual reconstruction
% ------------------------------------------------------


% tmp = '31_29'; 
% a = load(['/Users/longf/work/PHC_work/neuronmorphometry/res/hanchuan_rec/Neurolucida_reconstructed_neurons/swc_z_rescaled/c_', tmp, 'LHregion0.ASC.swc'], 'ASCII');
% b = load(['/Users/longf/work/PHC_work/neuronmorphometry/res/hanchuan_rec/pc_swc/', tmp, 'LHregion0.lsm.pc.swc'], 'ASCII');

tmp = '35_33';
% tmp = '43_41';
% tmp = '47_45';
% tmp = '51_49';

a = load(['/Users/longf/work/PHC_work/neuronmorphometry/res/hanchuan_rec/Neurolucida_reconstructed_neurons/swc_z_rescaled/c_', tmp, 'LHregion.ASC.swc'], 'ASCII');
b = load(['/Users/longf/work/PHC_work/neuronmorphometry/res/hanchuan_rec/pc_swc/', tmp, 'LHregion.lsm.pc.swc'], 'ASCII');

% root_a = find(a(:,7)==-1);
% root_b = find(b(:,7)==-1); %43_41
% % % root_b = size(b,1);
% % 
% dis = a(root_a,3:5)-b(root_b,3:5);
% 
% b(:,3:5) = b(:,3:5) + repmat(dis, [size(b,1),1]);
% 
% % b(:,5) = -b(:,5);
% % b(:,4) = b(:,4)-50;
% 
% % % b(:,3) = b(:,3)-15;
% 
% % b(:,4) = b(:,4)+20;

display_neuron_swcformat_FL(a, 1, [0 0 0]);

c = load('sigPoints2','ASCII');
c

% hold on;
% for i=1:length(c)
%      c(i)
% %     plot3(a(c(i)+1,3), a(c(i)+1,4), a(c(i)+1,5),'o','color', [1,0,0]); 
%     plot3(a(c(i),3), a(c(i),4), a(c(i),5),'o','color', [1,0,0]); 
% 
%     pause;
% end;
% 

display_neuron_swcformat_FL(b, 0, [0 0 1]);

c = load('sigPoints1','ASCII');
c
hold on;
for i=33:length(c)
    c(i)
%     plot3(b(c(i)+1,3), b(c(i)+1,4), b(c(i)+1,5),'o','color', [1,0,0]); 
    plot3(b(c(i),3), b(c(i),4), b(c(i),5),'o','color', [1,0,0]); 

    pause;
end;

% matchingList = load('/Users/longf/work/v3d_2.x/neuron_matching/res.txt', 'ASCII');

matchingList = load('/Users/longf/work/v3d_2.x/neuron_matching/res_auto_manual.txt', 'ASCII');
display_neuron_match(b, a, matchingList);




    