function exp_see_align_res(swcfile1, swcfile2, matchfile)
% visualize the neuron matching results
% by Hanchuan Peng
% 090209

a = load_neuron_swcfile(swcfile2);
b = load_neuron_swcfile(swcfile1);

% b(:,5) = b(:,5) + 20;
    
matchingList = load(matchfile, 'ASCII');

% display_neuron_match_nonmatch(b, a, matchingList, [0 0 1], [0 0 0], [0 1 1], [1 0 0]);
display_neuron_match_nonmatch(b, a, matchingList, [0 0 1], [0 0 0], [0 0 1], [0 0 0]);

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


% saveSwcVariable2File(d, '/Users/longf/work/v3d_2.x/neuron_matching/exp_result/different_neurons/diffneuron1_aligned.swc');

