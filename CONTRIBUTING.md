# Welcome fellow contributor!
Here you may find some guides for your work on this satellite.

## How to start?
First I have to tell you that this repository is write-only. So any contribution
need to be fed from the fork of this repository. A fork is a mirror repository where
you can develop your own stuff without bothering others' and after you finished your work
you can open back a pull request to merge your code to this repo.

To create a new fork click on the branch icon and wait.

## Git
Install [here](https://git-scm.com/downloads).

Also if you are on linux make sure that you assigned a text editor to git.
You can do it with the following command:
```bash
git config --global core.editor "nvim" # or your basic text editor of choice
```

### Setting up SSH authentication
In order to work from your computer with Git you must use SSH authentication.
First generate an SSH key like this:
```bash
ssh-keygen -t rsa -b 2048 -q -N ""
cat %USERPROFILE%/.ssh/id_rsa.pub
```

Theoretically this will output the public key which you should upload to your GitHub account.
See this [article](https://docs.github.com/en/authentication/connecting-to-github-with-ssh/adding-a-new-ssh-key-to-your-github-account) for details.

### Cloning
After you've created your fork repo and added SSH authentication, clone your fork:
```bash
git clone ssh://git@github.com/yourusername/yourCeleritasFork
```

### Publish your work
When you're finished a portion of your work, save it by committing.
Go to the cloned directory of your fork in your terminal then enter:
```bash
git add . # tell git you want to commit all your changes
git commit # start committing
```

This will open a text editor where you can type what you've changed.
The first line should always be a short title of your commit, where you summarize your change in max 1 sentence.

For example:
> optimised measurement loop

> fixed xy bug

> implemented a new subsystem (I hope we won't need this type of commit a lot)

Then explain the changes you've made detailing why the program benefits from them.
If this is a bug tell us what was it and how you fixed.
If your work has a trello card open (its likely to happen) its okay to say "for details look at the trello card" if you
are talking about something deep and well documented in Trello.

After you committed, push your work to GitHub to save it to the cloud.
```bash
git push origin main # or your branch if you know what you're doing
```

#### When to commit?
Ideally commit when you want to sync your code between your devices (if you develop across multiple...) or
when you finished with a well defineable part of your work (e.g. subtasks in trello).

### Update your local codebase

> It is important that you only sync when you're merged your stuff and doesn't have anything unfinished as there might be conflicts with the upstream repository.

As we're working in a team sometimes we need each others' changes.
First at your fork, sync your codebase with the upstream repository (it will be shown as a
yellow backgrounded section with a green button. Click it). After you've done with Github you should also pull the upstream changes to your local clone by the following command:
```bash
git pull origin main # or any other branch if you know what you're doing
```

Sometimes (due to STMCubeIDE generated files mostly) there are conflicts between your fork
on GitHub and your device. I'd say its a lot easier to just delete your local clone and clone it again.

## GitHub Apps
Here I'd say a few words about the GitHub apps around us that might help our work.

### GitHub Mobile App
> Link: [github.com/mobile](https://github.com/mobile)

GitHub has a mobile app for easier access on the go.
It has code navigation, issues, and full pull request support.

### GitHub Desktop
> Link: [github.com/apps/desktop](https://github.com/apps/desktop)

GitHub also released a helper desktop app that makes the work with Git a bit easier.
(spoiler: it has complete support for other github features like issues or prs as well...)
It is available on Windows so if you don't like the terminal you might want to take a look at it.

## About Pull Requests
Pull Requests (PRs) are the way GitHub handles the contributing. It's basically a request that asks the maintainer (now its me) to merge your changes into the upstream repository.
The maintainer then will ask others (or himself) to review your changes and if everything is okay merges your code.

### Starting the process
You can open a new PR from your fork when you have any commits that the upstream repo doesn't. (GitHub will automatically suggest that with the yellow backgrounded section with the green button.)

In the body of your PR, please introduce your changes shortly and if you are working basiced on a Trello card (and hopefully you do so) please link or mention your card.
However if you do not have a Trello card please also tell us why you've done your changes.

Then ask someone to review your code, make sure you did all the stuff needed and create the Pull Request.
Go to the top of the right panel and choose a (hopefully active) user to review your code.
He will be notified with an email about that.

### Reviews
When you open a PR you must ask someone (other than you!) who once again reviews your changes ensuring that you haven't introduced any bugs or other issue with your fix.

GitHub has a [good article](https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/reviewing-changes-in-pull-requests/reviewing-proposed-changes-in-a-pull-request) about the review process so check it out if you find yourself in the reviewer's position.

### Merge
After had your review and fixed everything that reviewer suggested (discussed it with him) please notify me and I will merge your code within a day.
After the merge your code is offically part of the Celeritas codebase. Cheers!
