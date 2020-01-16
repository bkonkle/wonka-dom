module.exports = {
  extends: ['eslint:recommended', 'prettier'],
  parser: 'babel-eslint',
  env: {
    node: true,
    jest: true,
  },
  ignorePatterns: ['*.bs.js'],
}
