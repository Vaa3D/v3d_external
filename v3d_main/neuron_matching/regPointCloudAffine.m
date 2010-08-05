function Pos_subject_new = regPointCloudAffine(Pos_subject, Pos_cpt_subject, Pos_cpt_target) 
% function Pos_subject_new = regPointCloudAffine(Pos_subject, Pos_cpt_subject, Pos_cpt_target) 
% 
%
% point cloud registration using global affine transform
% Pos_subject: (point number) x 3 x (stack number) in the subject
% Pos_cpt_subject: (control point number) x 3 x (stack number) in the subject
% Pos_cpt_target: (control point number) x 3 x 1 in the target
%
% F. Long
% 20081014

%------------------------
% initialization
%------------------------

stacknum = size(Pos_subject,3);
cellnum = size(Pos_subject,1);

if size(Pos_cpt_subject,1)~=size(Pos_cpt_target,1)
    printf('number of controling points in sugject and target do not match\n');
    Pos_subject_new = [];
    return;
end;

cellnum_cpt = size(Pos_cpt_subject,1);

Pos_subject_new = Pos_subject;
Pos_subject_new(:) = 0;

for subject = [1:stacknum]
   
    % generate subject points, remove those with x,y,z=0

%     subjectPts = zeros(cellnum,3);

    ind = setdiff([1:cellnum], find(sum(Pos_subject(:,:,subject),2)==0));
    subjectPts = squeeze(Pos_subject(ind, :, subject)); % x,y,z

    % generate control points, must have one-to-one correspondence
    % between subject and target stacks 
    targetCpt = [];
    subjectCpt = [];

    ind2 = setdiff([1:cellnum_cpt], find((sum(squeeze(Pos_cpt_subject(:,:,subject)),2)==0)|(sum(Pos_cpt_target,2)==0)));

    targetCpt = squeeze(Pos_cpt_target(ind2, :)); % x,y,z
    subjectCpt = squeeze(Pos_cpt_subject(ind2, :, subject)); % x,y,z


    % global affine transform

    M = size(subjectCpt,1);
    X = [subjectCpt, ones(M,1)];
    U = targetCpt;
    K = size(targetCpt,1);

%     % % method 1, not right
%     T = X\U;
% 
%     len = size(T,2);
%     T(:,len+1) = [zeros(1,len) 1]';
% 
%     P = X*T;
%     P(:,1:end-1)-U


    % method 2
    Tinv = X\U; % it should not be call INVT, it is T itself

    len = size(Tinv,2);
    Tinv(:,len+1) = [zeros(1,len) 1]';

    T = inv(Tinv);
    T(:,len+1) = [zeros(1,len) 1]';

    tform1 = maketform('affine', T); % T is in fact backward transform
    tform2 = maketform('affine', Tinv); % Tinv is in fact forward transform

    transPtsTmp = [];
    [transPtsTmp(:,1), transPtsTmp(:,2), transPtsTmp(:,3)] = tformfwd(tform2, subjectPts(:,1), subjectPts(:,2), subjectPts(:,3)); % in the order of x,y,z

    transPts = zeros(cellnum,3);
    transPts(ind,:) = transPtsTmp;

    Pos_subject_new(:,:,subject) = transPts;
    
      
end;
